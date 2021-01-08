// UnitControl © 2019 Niklaus Houska. All Rights Reserved 

#include "UnitControl.h"
#include "Formation.h"

void UFormation::SetOrigin(FVector2D NewOrigin)
{
	Origin = NewOrigin;
}

void UFormation::SetOrientation(FVector2D NewOrientation)
{
	NewOrientation.Normalize();
	Orientation = NewOrientation;
}

void UFormation::SetSpacing(float NewSpacing)
{
	if (NewSpacing < 45.0f)
	{
		Spacing = 45.0f;
		return;
	}
	Spacing = NewSpacing;
}

void UFormation::Precompute()
{
	FormationBasisVectorA = Orientation * Spacing;
	FormationBasisVectorB = FormationBasisVectorA.GetRotated(270.0f);

	float Determinant = 1.0f / (FormationBasisVectorA.X * FormationBasisVectorB.Y - FormationBasisVectorA.Y * FormationBasisVectorB.X);

	InvFormationBasisVectorA = Determinant * FVector2D(FormationBasisVectorB.Y, -FormationBasisVectorA.Y);
	InvFormationBasisVectorB = Determinant * FVector2D(-FormationBasisVectorB.X, FormationBasisVectorA.X);

	float CenterBias = ((float)GetColumnCount() - 1.0f) / 2.0f;
	FormationTranslation = Origin - CenterBias * FormationBasisVectorB;
}

FVector2D UFormation::GetOrigin() const
{
	return Origin;
}

TArray<FVector> UFormation::GetFirstXRows(int32 Num) const
{
	TArray<FVector> Result;

	if (Num < 0)
	{
		Num = 0;
	}
	if (Num > 100)
	{
		Num = 100;
	}

	
	for (int32 I = 0; I < Num; I++)
	{
		for (int32 J = 0; J < GetColumnCount(I); J++)
		{
			Result.Add(GetWorldLocation(FVector2D(I, J)));
		}
	}
	return Result;
}

int32 UFormation::GetColumnCount(int32 Row) const
{
	return 1;
}

float UFormation::GetSpacing() const
{
	return Spacing;
}

FVector2D UFormation::GetOrientation() const
{
	return Orientation;
}

float UFormation::GetLocationScore(const FVector & MyLocation, const FVector & WorldLocation) const
{
	FVector2D FormationLocation = GetFormationLocation(WorldLocation);
	FVector2D ClosestFormationLoc = GetClosestLocation(MyLocation);
	float RowScore = (ClosestFormationLoc.X - FormationLocation.X) * 10.0f;
	float ColumnScore = 0.0;

	float ColDiff = ClosestFormationLoc.Y - FormationLocation.Y;
	if (FMath::Abs(ColDiff) < 0.01f)
	{
		ColumnScore = 5.0f;
	}
	else
	{
		ColumnScore = 1.0f / ColDiff;
	}
	//float RowScore = FormationLocation.X * GetColumnCount(FormationLocation.X) + 1.0f;
	//float CenterBias = ((float)GetColumnCount() - 1.0f) / 2.0f;
	//float ColumnScore = FMath::Abs(FormationLocation.Y - CenterBias);
	return RowScore + ColumnScore;
}



FVector2D UFormation::GetClosestLocation(const FVector & FromWorldLocation) const
{
	const FVector2D FormationLocation = GetFormationLocation(FromWorldLocation) + FVector2D(-0.01, -0.01);
	FVector2D ClosestLocation;
	if (FormationLocation.X < 0.0f)
	{
		ClosestLocation.X = 0.0f;
	}
	else
	{
		float FloorX = FMath::FloorToFloat(FormationLocation.X);
		float FloorX1 = FMath::FloorToFloat(FormationLocation.X + 1.0f);
		if (FloorX1 - FloorX > 1.01f)
		{
			UE_LOG(LogTemp, Warning, TEXT("There is an precision error when finding closest location"));
		}
		ClosestLocation.X = FormationLocation.X - FloorX < 0.5f ? FloorX : FloorX1;
	}
	
	if (FormationLocation.Y < 0.0f)
	{
		ClosestLocation.Y = 0.0f;
	}
	else if (FormationLocation.Y > GetColumnCount(ClosestLocation.X) - 1.0f)
	{
		ClosestLocation.Y = GetColumnCount(ClosestLocation.X) - 1.0f;

	}
	else
	{
		float FloorY = FMath::FloorToFloat(FormationLocation.Y);
		float FloorY1 = FMath::FloorToFloat(FormationLocation.Y + 1.0f);
		if (FloorY1 - FloorY > 1.01f)
		{
			UE_LOG(LogTemp, Warning, TEXT("There is an precision error when finding closest location"));
		}
		ClosestLocation.Y = FormationLocation.Y - FloorY < 0.5f ? FloorY : FloorY1;
	}
	return ClosestLocation;
}

bool UFormation::IsLocationAbove(const FVector & BelowWorldLocation, const FVector & AboveWorldLocation, float Tolerance) const
{
	FVector2D BelowFormationLocation = GetFormationLocation(BelowWorldLocation);
	FVector2D AboveFormationLocation = GetFormationLocation(AboveWorldLocation);
	return (BelowFormationLocation.X - AboveFormationLocation.X) > (Tolerance / Spacing);
}

bool UFormation::IsLocationLeft(const FVector & RightWorldLocation, const FVector & LeftWorldLocation, float Tolerance) const
{
	FVector2D RightFormationLocation = GetFormationLocation(RightWorldLocation);
	FVector2D LeftFormationLocation = GetFormationLocation(LeftWorldLocation);
	return (LeftFormationLocation.Y - RightFormationLocation.Y) > (Tolerance / Spacing);
}

TArray<FVector> UFormation::GetWorldLocations(const TArray<FVector2D>& inFormationLocations) const
{
	TArray<FVector> Result;
	for (FVector2D FormationLocation : inFormationLocations)
	{
		Result.Add(GetWorldLocation(FormationLocation));
	}
	return Result;
}

FVector UFormation::GetWorldLocation(const FVector2D & FormationPosition) const
{
	float WorldX = FormationPosition.X * FormationBasisVectorA.X + FormationPosition.Y * FormationBasisVectorB.X + FormationTranslation.X;
	float WorldY = FormationPosition.X * FormationBasisVectorA.Y + FormationPosition.Y * FormationBasisVectorB.Y + FormationTranslation.Y;
	return FVector(WorldX, WorldY, 0.0f);
}

FVector2D UFormation::GetFormationLocation(const FVector & WorldPosition) const
{
	FVector2D Translated = FVector2D(WorldPosition.X, WorldPosition.Y);
	Translated -= FormationTranslation;

	float FormationX = Translated.X * InvFormationBasisVectorA.X + Translated.Y * InvFormationBasisVectorB.X;
	float FormationY = Translated.X * InvFormationBasisVectorA.Y + Translated.Y * InvFormationBasisVectorB.Y;

	return FVector2D(FormationX, FormationY);
}

bool UFormation::IsAValidFormationLocation(const FVector & WorldLocation, float Tolerance) const
{
	FVector2D ClosestLocation = GetClosestLocation(WorldLocation);
	return FVector2D::Distance(GetFormationLocation(WorldLocation), ClosestLocation) < Tolerance / Spacing;
}