// UnitControl © 2019 Niklaus Houska. All Rights Reserved

#pragma once

#include "GameFramework/PawnMovementComponent.h"
#include "RobotMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class UNITCONTROL_API URobotMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()
	
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void GetNonHolonomicControls(FVector HolonomicVelocity, FRotator& NewRotator, float& LinearSpeed, float DeltaTime) const;

	const float MaxAngularSpeed = 3.96f; // [rad/s]

	const float T = 0.2f; // Time to reach orientation
	
public:
	bool bIsDebug;
	
};
