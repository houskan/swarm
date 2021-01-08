// UnitControl © 2019 Niklaus Houska. All Rights Reserved 

#pragma once

#include "UObject/NoExportTypes.h"
#include "Formation.generated.h"

/**
 * 
 */
UCLASS(abstract)
class UNITCONTROL_API UFormation : public UObject
{
	GENERATED_BODY()

public:

	virtual void SetOrigin(FVector2D NewOrigin);

	virtual void SetOrientation(FVector2D NewOrientation);

	virtual void SetSpacing(float NewSpacing);

	virtual void Precompute();
	
	virtual FVector2D GetOrigin() const;

	virtual TArray<FVector> GetFirstXRows(int32 Num) const;

	virtual int32 GetColumnCount(int32 Row = 0) const;

	virtual float GetSpacing() const;

	virtual FVector2D GetOrientation() const;

	virtual float GetLocationScore(const FVector & MyLocation, const FVector & WorldLocation) const;

	virtual FVector2D GetClosestLocation(const FVector & FromWorldLocation) const;

	virtual bool IsLocationAbove(const FVector & BelowWorldLocation, const FVector & AboveWorldLocation, float Tolerance = 10.0f) const;
	virtual bool IsLocationLeft(const FVector & RightWordlLocation, const FVector &LeftWorldLocation, float Tolerance = 10.0f) const;

	virtual TArray<FVector> GetWorldLocations(const TArray<FVector2D> &inFormationLocations) const;

	virtual FVector GetWorldLocation(const FVector2D & FormationPosition) const;

	virtual FVector2D GetFormationLocation(const FVector & WorldPosition) const;

	virtual bool IsAValidFormationLocation(const FVector & WorldLocation, float Tolerance = 10.0f) const;

protected:

	FVector2D Origin = FVector2D::ZeroVector;

	FVector2D Orientation = FVector2D(1.0f, 0.0f);
	
	float Spacing = 1.0f;

	FVector2D FormationTranslation = FVector2D::ZeroVector;

	FVector2D FormationBasisVectorA = FVector2D(1.0f, 0.0f);
	FVector2D FormationBasisVectorB = FVector2D(0.0f, 1.0f);

	FVector2D InvFormationBasisVectorA = FVector2D(1.0f, 0.0f);
	FVector2D InvFormationBasisVectorB = FVector2D(0.0f, 1.0f);

};
