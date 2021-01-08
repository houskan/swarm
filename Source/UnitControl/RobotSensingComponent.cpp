// UnitControl © 2019 Niklaus Houska. All Rights Reserved

#include "UnitControl.h"
#include "RobotPawn.h"
#include "RobotSensingComponent.h"


URobotSensingComponent::URobotSensingComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SensingInterval = 0.1f;
	SightRadius = 800.0f;
	SetPeripheralVisionAngle(180.0f);
	bOnlySensePlayers = false;
	bHearNoises = false;
	bSeePawns = true;
}

bool URobotSensingComponent::ShouldCheckVisibilityOf(APawn * Pawn) const
{
	return dynamic_cast<ARobotPawn*>(Pawn) != nullptr;
}

void URobotSensingComponent::UpdateAISensing()
{
	const AActor* const Owner = GetOwner();
	if (!IsValid(Owner) || (Owner->GetWorld() == NULL))
	{
		return;
	}

	TArray<TWeakObjectPtr<ARobotPawn> > NewRobots;
	for (FConstPawnIterator Iterator = Owner->GetWorld()->GetPawnIterator(); Iterator; Iterator++)
	{
		ARobotPawn* const TestPawn = Cast<ARobotPawn>(*Iterator);
		if (!IsSensorActor(TestPawn) && ShouldCheckVisibilityOf(TestPawn))
		{
			if (CouldSeePawn(TestPawn, false) && HasLineOfSightTo(TestPawn))
			{
				NewRobots.AddUnique(TestPawn);
				SensePawn(**Iterator);
			}
		}
	}
	FVector Origin = Owner->GetActorLocation();
	NewRobots.Sort([Origin](const TWeakObjectPtr<ARobotPawn>& LHS, const TWeakObjectPtr<ARobotPawn>& RHS)
	{
		if (!LHS.IsValid() || !RHS.IsValid()) return false;
		return FVector::Dist(Origin, LHS->GetActorLocation()) < FVector::Dist(Origin, RHS->GetActorLocation());
	});

	SensedRobots = NewRobots;

}

/**
 * @return: List of valid pointers to currently sensed robots
 */
TArray<TWeakObjectPtr<ARobotPawn>> URobotSensingComponent::GetSensedRobots()
{
	SensedRobots.RemoveAll([](const TWeakObjectPtr<ARobotPawn> Robot)
	{
		return !Robot.IsValid();
	});
	return SensedRobots;
}
