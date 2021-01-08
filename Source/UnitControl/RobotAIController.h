// UnitControl © 2019 Niklaus Houska. All Rights Reserved 

#pragma once

#include "AIController.h"
#include "RobotAIController.generated.h"

class UFormation;

UCLASS()
class UNITCONTROL_API ARobotAIController : public AAIController
{
	GENERATED_BODY()
	
public:

	//Begin AActor Interface
	virtual void Tick(float DeltaSeconds) override;
	//End AActor Interface

	//Begin Controller Interface
	virtual void Possess(APawn* inPawn) override;
	virtual void UnPossess() override;
	//End Controller Interface

	void SetCurrentFormation(UFormation* InFormation);
	void ActivateRobot(bool bActivate);

	UFormation* GetFormation() const { return CurrentFormation; }
	bool IsActivated() const { return bIsActivated; }

protected:
	UFormation* CurrentFormation;
	bool bIsActivated = false; // Master Switch for the Pawn
};
