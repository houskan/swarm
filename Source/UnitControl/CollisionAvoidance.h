// UnitControl © 2019 Niklaus Houska. All Rights Reserved 

#pragma once

#include "UObject/NoExportTypes.h"
#include "CollisionAvoidance.generated.h"

class ARobotPawn;
/**
 * 
 */
UCLASS()
class UNITCONTROL_API UCollisionAvoidance : public UObject
{
	GENERATED_BODY()
	
public:

	FVector CalculataCollisionObstacle(FVector MyLocation, FVector OtherLocation, FVector MyVelocity, FVector OtherVeloctiy, float MutualityFactor = 0.5f) const;
	bool CalculateAllowedVelocities(TArray<FVector2D> & ResultPolygon, float MyYaw, const TArray<FVector> & CollisionObstacles) const;
	FVector GetOptimalHolonomicVelocity(const TArray<FVector2D>& AllowedVelocityPolygon, const FVector& PreferedVelocity) const;

	void PrecomputePAHV();

protected:
	const float Radius = 30.0f; //[cm]
	const float Horizon = 1.0f; // [s] no collision is calculated up to horizon
	const float MaxVelocity = 110.0f; //[cm/s]

	TArray<FVector2D> P_AHV;

	bool IsPointInHalfplane(const FVector & Halfplane, const FVector2D & Point) const;
	bool TwoLineIntersection(FVector2D & IntersectionPoint, const FVector & L1, const FVector & L2) const;
	bool SegmentLineIntersection(FVector2D & IntersectionPoint, const FVector2D & P1, const FVector2D & P2, const FVector & Line) const;
	void ClosestPointOnSegment(FVector2D & ClosestPoint, const FVector2D & P1, const FVector2D & P2, const FVector2D Point) const;

};
