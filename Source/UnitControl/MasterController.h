// UnitControl © 2019 Niklaus Houska. All Rights Reserved 

#pragma once

#include "GameFramework/PlayerController.h"
#include "MasterController.generated.h"

class AInfo_HUD;
class UFormation;
class ARobotPawn;
/**
 * 
 */
UCLASS()
class UNITCONTROL_API AMasterController : public APlayerController
{
	GENERATED_BODY()

public:

	AMasterController();

	virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

	virtual void SetupInputComponent() override;

	//Cashed pointer to HUD for quicker access
	AInfo_HUD* HUDPtr;
	

protected:

	UFUNCTION()
	void MoveFormationPressed();

	UFUNCTION()
	void MoveFormationReleased();

	UFUNCTION()
	void SpawnRobotPressed();

	UFUNCTION()
	void DeleteRobotPressed();

	UFUNCTION()
	void ToggleColumnCount();

	UFUNCTION()
	void ToggleFormation();

	UFUNCTION()
	void ToggleSpacing();

	UFUNCTION()
	void Debug();

	UPROPERTY()
	UFormation* CurrentFormation;

	FVector InitialWorldLocation;
	FVector CurrentWorldLocation;

	TArray<TWeakObjectPtr<ARobotPawn>> ListOfRobots;

	bool bMapStarted = false;

	void UpdateFormation();

	FTimerHandle MyTimeHandle;
	void Initialize();

	bool bAllAreResting = false;
};
