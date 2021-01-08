// UnitControl © 2019 Niklaus Houska. All Rights Reserved

#include "UnitControl.h"
#include "RobotMovementComponent.h"
#include "RobotPawn.h"

void URobotMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction * ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime))
	{
		return;
	}

	FVector DesiredMovementThisFrame = ConsumeInputVector();
	DesiredMovementThisFrame.Z = 0.0f;

	if (DesiredMovementThisFrame.IsNearlyZero())
	{
		return;
	}

	FRotator AcceptedRotator;
	float DesiredSpeed;

	GetNonHolonomicControls(DesiredMovementThisFrame, AcceptedRotator, DesiredSpeed, DeltaTime);
	
	FHitResult Hit;
	Velocity = UpdatedComponent->GetForwardVector() * DesiredSpeed * DeltaTime;
	SafeMoveUpdatedComponent(Velocity, AcceptedRotator, true, Hit);

	if (Hit.IsValidBlockingHit())
	{
		SlideAlongSurface(Velocity, 1.0f - Hit.Time, Hit.Normal, Hit);
	}
}


void URobotMovementComponent::GetNonHolonomicControls(FVector HolonomicVelocity, FRotator& NewRotator, float& LinearSpeed, float DeltaTime) const
{
	FRotator CurrentRotation = UpdatedComponent->GetComponentRotation();
	FVector NonHolonomicVelocity = FVector::ZeroVector;

	// Get the desired Angular Speed and allowed linear Velocity
	float Phi = HolonomicVelocity.Rotation().Yaw - CurrentRotation.Yaw;
	
	if (Phi > 181.0f)
		Phi -= 360.0f;
	else if (Phi < -179.0f)
		Phi += 360.0f;

	float PhiRad = FMath::DegreesToRadians(Phi);
	
	float AngularSpeed = PhiRad / T; // [rad/s]
	
	if (FMath::Abs(PhiRad) < 0.01f || FMath::Abs(PhiRad - PI) < 0.01f)
		NonHolonomicVelocity = HolonomicVelocity;
	else
		NonHolonomicVelocity = HolonomicVelocity * (PhiRad * FMath::Sin(PhiRad)) / (2.0f * (1.0f - FMath::Cos(PhiRad)));

	
	LinearSpeed = FMath::Abs(AngularSpeed) > MaxAngularSpeed ? 0.0f : NonHolonomicVelocity.Size();
	
	// Get the new orientation
	float AngularSpeedDeg = Phi / T; // [deg/s]
	Phi = NonHolonomicVelocity.Rotation().Yaw - CurrentRotation.Yaw;

	if (Phi > 181.0f)
		Phi -= 360.0f;
	else if (Phi < -179.0f)
		Phi += 360.0f;

	NewRotator = CurrentRotation;
	if (Phi > 0.0f)
	{
		if (Phi > DeltaTime * AngularSpeedDeg)
			NewRotator.Yaw += DeltaTime * AngularSpeedDeg;
		else
			NewRotator = NonHolonomicVelocity.Rotation();
	}
	else if (Phi < 0.0f)
	{
		if (Phi < DeltaTime * AngularSpeedDeg)
			NewRotator.Yaw += DeltaTime * AngularSpeedDeg;
		else
			NewRotator = NonHolonomicVelocity.Rotation();
	}
}