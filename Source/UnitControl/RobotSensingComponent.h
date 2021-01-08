// UnitControl © 2019 Niklaus Houska. All Rights Reserved

#pragma once

#include "Perception/PawnSensingComponent.h"
#include "RobotSensingComponent.generated.h"

class ARobotPawn;
/**
 * 
 */
UCLASS()
class UNITCONTROL_API URobotSensingComponent : public UPawnSensingComponent
{
	GENERATED_UCLASS_BODY()
	
public:

	// Begin PawnSensingComponent interface
	virtual bool ShouldCheckVisibilityOf(APawn* Pawn) const override;
	virtual void UpdateAISensing() override;
	// End PawnSensingComponent interface

	TArray<TWeakObjectPtr<ARobotPawn>> GetSensedRobots();

	TArray<TWeakObjectPtr<ARobotPawn>> SensedRobots;
	
};
