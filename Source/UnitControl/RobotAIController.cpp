// UnitControl © 2019 Niklaus Houska. All Rights Reserved 

#include "UnitControl.h"
#include "Formation.h"
#include "ColumnFormation.h"
#include "RobotAIController.h"


void ARobotAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ARobotAIController::Possess(APawn * inPawn)
{
	Super::Possess(inPawn);
	SetActorTickEnabled(true);
}

void ARobotAIController::UnPossess()
{
	SetActorTickEnabled(false);
	Super::UnPossess();
}

void ARobotAIController::SetCurrentFormation(UFormation* InFormation)
{
	CurrentFormation = InFormation;
}

void ARobotAIController::ActivateRobot(bool bActivate)
{
	bIsActivated = bActivate;
}
