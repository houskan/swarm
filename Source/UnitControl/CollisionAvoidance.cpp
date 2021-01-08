// UnitControl © 2019 Niklaus Houska. All Rights Reserved 

#include "UnitControl.h"
#include "CollisionAvoidance.h"


FVector UCollisionAvoidance::CalculataCollisionObstacle(FVector MyLocation, FVector OtherLocation, FVector MyVelocity, FVector OtherVeloctiy, float MutualityFactor) const
{

	FVector2D V_Opt_A = FVector2D(MyVelocity);
	FVector2D V_Opt_B = FVector2D(OtherVeloctiy);


	//Calculate the two lines defining the collision obstacle;
	FVector2D CenterB = FVector2D(OtherLocation - MyLocation);
	float RadiusB = 2.0f * Radius;

	CenterB /= Horizon;
	RadiusB /= Horizon;

	float RadiusA = FMath::Sqrt(CenterB.SizeSquared() - FMath::Square(RadiusB));
	float Distance = CenterB.Size();
	FVector2D E = CenterB / Distance;
	float X = (FMath::Square(RadiusA) - FMath::Square(RadiusB) + FMath::Square(Distance)) / (2.0f * Distance);
	float Y = FMath::Sqrt(FMath::Square(RadiusA) - FMath::Square(X));

	FVector2D P1 = FVector2D(X * E.X - Y * E.Y, X * E.Y + Y * E.X);
	FVector2D P2 = FVector2D(X * E.X + Y * E.Y, X * E.Y - Y * E.X);

	FVector2D RelativeVelocity = V_Opt_A - V_Opt_B;
	FVector2D PointOnBoundary;

	// Find closest point on boundary by checking both lines and the bounding circle
	bool bRelativeVelocityIsInside = true;

	FVector2D P1_FarAway = 100.0f * P1;
	ClosestPointOnSegment(PointOnBoundary, P1, P1_FarAway, RelativeVelocity);

	float MinDist = FVector2D::Distance(PointOnBoundary, RelativeVelocity);
	FVector2D CandidatePoint;

	FVector2D P2_FarAway = 50.0f * P2;
	ClosestPointOnSegment(CandidatePoint, P2, P2_FarAway, RelativeVelocity);

	FVector2D InsideTestPoint;
	ClosestPointOnSegment(InsideTestPoint, PointOnBoundary, CandidatePoint, RelativeVelocity);

	if (FVector2D::Distance(PointOnBoundary, InsideTestPoint) < 0.0001f || FVector2D::Distance(CandidatePoint, InsideTestPoint) < 0.0001f)
	{
		bRelativeVelocityIsInside = false;
	}

	if (FVector2D::Distance(CandidatePoint, RelativeVelocity) < MinDist)
	{
		MinDist = FVector2D::Distance(CandidatePoint, RelativeVelocity);
		PointOnBoundary = CandidatePoint;
	}

	if (FVector2D::Distance(PointOnBoundary, P1) < 0.0001f || FVector2D::Distance(PointOnBoundary, P2) < 0.0001f)
	{
		float DistToHorizonCircleCenter = FVector2D::Distance(CenterB, RelativeVelocity);
		MinDist = FMath::Abs(DistToHorizonCircleCenter - RadiusB);
		bRelativeVelocityIsInside = DistToHorizonCircleCenter < RadiusB;
		FVector2D DirNormal = bRelativeVelocityIsInside ? RelativeVelocity - CenterB : CenterB - RelativeVelocity;
		DirNormal.Normalize();
		PointOnBoundary = RelativeVelocity + MinDist * DirNormal;
	}

	FVector2D U = PointOnBoundary - RelativeVelocity;
	FVector2D N = U;
	N.Normalize();

	float Sign = bRelativeVelocityIsInside ? -1.0f : 1.0f;

	float A = Sign * N.X;
	float B = Sign * N.Y;
	float C = Sign * FVector2D::DotProduct(V_Opt_A + MutualityFactor * U, N);

	return FVector(A,B,C);
}

bool UCollisionAvoidance::CalculateAllowedVelocities(TArray<FVector2D>& ResultPolygon, float MyYaw, const TArray<FVector>& CollisionObstacles) const
{
	ResultPolygon.Empty();

	if (P_AHV.Num() < 3)
		return false;

	TArray<FVector2D> PointSet;
	for (FVector2D Point : P_AHV)
		PointSet.Add(Point.GetRotated(MyYaw));

	for (FVector CollisionObstacle : CollisionObstacles)
	{
		int32 NumPoints = PointSet.Num();
		int32 Count = 0;
		int32 Index1 = -1;
		int32 Index2 = -1;
		FVector2D Intersec1 = FVector2D::ZeroVector;
		FVector2D Intersec2 = FVector2D::ZeroVector;

		for (int32 I = 0; I < NumPoints; I++)
		{
			int32 J = I == NumPoints - 1 ? 0 : I + 1;
			FVector2D Intersection;
			if (SegmentLineIntersection(Intersection, PointSet[I], PointSet[J], CollisionObstacle))
			{
				if (Count == 0)
				{
					Index1 = J;
					Intersec1 = Intersection;
					Count++;
				}
				else if (Count == 1)
				{
					Index2 = J;
					Intersec2 = Intersection;
					Count++;
				}
				else
				{
					Intersec2 = Intersection;
					Index2 = J;
				}
			}
		}

		if (Count == 0 || Count == 1)
		{
			if (IsPointInHalfplane(CollisionObstacle, PointSet[0]))
				continue;
			else
				return false;
		}
		else if (Count == 2)
		{
			PointSet.Insert(Intersec1, Index1);
			if (Index2 > Index1) Index2++;
			PointSet.Insert(Intersec2, Index2);
			
			PointSet.RemoveAll([this, &CollisionObstacle](const FVector2D& Point) { return !IsPointInHalfplane(CollisionObstacle, Point); });
			if (PointSet.Num() < 3)
				return false;
		}
	}
	ResultPolygon = PointSet;
	return true;
}

FVector UCollisionAvoidance::GetOptimalHolonomicVelocity(const TArray<FVector2D>& AllowedVelocityPolygon, const FVector & PreferedVelocity) const
{
	int32 N = AllowedVelocityPolygon.Num();
	if (N < 3)
		return FVector::ZeroVector;
	FVector2D VPref = FVector2D(PreferedVelocity);
	int8 PrevIndicator = 0;
	int8 CurrIndicator;

	bool bIsInside = true;

	for (int32 I = 0; I < N; I++)
	{
		FVector2D P1 = AllowedVelocityPolygon[I];
		FVector2D P2 = AllowedVelocityPolygon[(I + 1) % N];

		FVector2D AffineSegment = P2 - P1;
		FVector2D AffinePoint = VPref - P1;

		float CrossProd = FVector2D::CrossProduct(AffineSegment, AffinePoint);

		if (CrossProd < -0.0001f) CurrIndicator = 2;
		else if (CrossProd > 0.0001f) CurrIndicator = 1;
		else CurrIndicator = 0;

		if (CurrIndicator == 0)
		{
			bIsInside = true;
			break;
		}
		else if (PrevIndicator == 0)
		{
			PrevIndicator = CurrIndicator;
		}
		else if (PrevIndicator != CurrIndicator)
		{
			bIsInside = false;
		}
	}

	if (bIsInside)
	{
		return PreferedVelocity;
	}

	float MinDist = 100000.0f;
	FVector2D OptVel;
	for (int32 I = 0; I < N; I++)
	{
		FVector2D P1 = AllowedVelocityPolygon[I];
		FVector2D P2 = AllowedVelocityPolygon[(I + 1) % N];
		FVector2D CandidatePoint;
		float Distance;
		ClosestPointOnSegment(CandidatePoint, P1, P2, VPref);
		Distance = FVector2D::Distance(CandidatePoint, VPref);
		if (Distance < MinDist)
		{
			MinDist = Distance;
			OptVel = CandidatePoint;
		}
	}

	return FVector(OptVel.X, OptVel.Y, 0.0f);
}

void UCollisionAvoidance::PrecomputePAHV()
{
	TArray<float> Angles;
	Angles.Add(20); Angles.Add(60); Angles.Add(100); Angles.Add(140); Angles.Add(180.0f);
	Angles.Add(-140.0f); Angles.Add(-100.0f); Angles.Add(-60.0f); Angles.Add(-20);

	P_AHV.Empty(9);
	const FVector2D StraightVelocity = FVector2D(110.0f, 0.0f);
	for (float Angle : Angles)
	{
		P_AHV.Add(StraightVelocity.GetRotated(Angle));
	}
}

bool UCollisionAvoidance::IsPointInHalfplane(const FVector & Halfplane, const FVector2D & Point) const
{
	return (Halfplane.X * Point.X + Halfplane.Y * Point.Y - Halfplane.Z) < 0.001f;
}

bool UCollisionAvoidance::TwoLineIntersection(FVector2D & IntersectionPoint, const FVector & L1, const FVector & L2) const
{
	float Det = L1.X * L2.Y - L2.X * L1.Y;
	if (FMath::IsNearlyZero(Det))
		return false;

	IntersectionPoint.X = (L2.Y * L1.Z - L1.Y * L2.Z) / Det;
	IntersectionPoint.Y = (L1.X * L2.Z - L2.X * L1.Z) / Det;
	return true;
}

bool UCollisionAvoidance::SegmentLineIntersection(FVector2D & IntersectionPoint, const FVector2D & P1, const FVector2D & P2, const FVector & Line) const
{
	FVector SegmentAsLine = FVector(P2.Y - P1.Y, P1.X - P2.X, (P2.Y - P1.Y) * P1.X + (P1.X - P2.X) * P1.Y);
	if (!TwoLineIntersection(IntersectionPoint, SegmentAsLine, Line))
		return false;

	float DirX = P2.X - P1.X;
	float T = 0.0f;
	if (FMath::IsNearlyZero(DirX))
	{
		float DirY = P2.Y - P1.Y;
		T = (IntersectionPoint.Y - P1.Y) / DirY;
	}
	else
	{
		T = (IntersectionPoint.X - P1.X) / DirX;
	}
	return !(T < -0.0f || T > 1.0f);
}

void UCollisionAvoidance::ClosestPointOnSegment(FVector2D & ClosestPoint, const FVector2D & P1, const FVector2D & P2, const FVector2D Point) const
{
	float Denomin = FVector2D::DotProduct(P2 - P1, P2 - P1);
	if (FMath::IsNearlyZero(Denomin))
	{
		ClosestPoint = FVector2D::ZeroVector;
		return;
	}

	float T = FVector2D::DotProduct(Point - P1, P2 - P1) / Denomin;
	if (T < -0.0f)
		ClosestPoint = P1;
	else if (T > 1.0f)
		ClosestPoint = P2;
	else
		ClosestPoint = P1 + T * (P2 - P1);
}
