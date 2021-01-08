// UnitControl © 2019 Niklaus Houska. All Rights Reserved

#pragma once

#include "GameFramework/Pawn.h"
#include "RobotPawn.generated.h"

class URobotMovementComponent;
class URobotSensingComponent;
class ARobotAIController;
class UCollisionAvoidance;

namespace EStatusIndicator
{
	enum Status
	{
		Content_FullRow = 0,
		Content_SpaceOnBoth = 1,
		Content_SpaceOnRight = 2,
		Content_SpaceOnLeft = 3,
		Moving = 4,
		Content_UnsureBoth = 5,
		Content_UnsureRight = 6,
		Content_UnsureLeft = 7
	};
}

UCLASS()
class UNITCONTROL_API ARobotPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ARobotPawn();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	virtual void EndPlay(const EEndPlayReason::Type EEndPlayReason) override;

	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	//Robot Components
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* StaticMeshComp;
	
	UPROPERTY(VisibleAnywhere, Category = "Components")
	URobotMovementComponent* MovementComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	URobotSensingComponent* SensingComp;

	UPROPERTY()
	UCollisionAvoidance* CollisionAvoidanceComp;

	UFUNCTION()
	void OnPawnSeen(APawn* SeenPawn);

	
	//Public functions to access shared info
	FVector2D GetGoalLocation() const { return GoalLocation; }
	FVector GetHolonomicVelocity() const { return HolonomicVelocity; }
	EStatusIndicator::Status GetStatusIndicator() const { return MyStatusIndicator; }
	TWeakObjectPtr<ARobotPawn> GetLink(int32 LinkIndex) const { return NeighborLinks[LinkIndex]; }
	int32 GetMovementStrategy() const { return MovementStrategy; }
	bool IsLeftFree() const { return bLeftIsFree;  }
	bool IsRightFree() const { return bRightIsFree; }
	bool IsTopFree() const { return bTopIsFree; }

	void PrintDebugInfo();
	void SetGoalLocation(FVector2D InGoalLocation);

protected:

	UFUNCTION()
	void UpdateGoalLocation();

	bool GetImprovedLocation(FVector2D & ImprovedLocation, FVector2D & ImprovedLocation2, bool bHavePotentialLeftLink, bool bHavePotentialRightLink);
	FVector2D GetBackUpLocation();
	bool IsLocationConflicted(const FVector2D & FormationLocation,const TArray<TWeakObjectPtr<ARobotPawn>> &DominantRobots, bool bCheckNeighborInvariants = true) const;
	float GetForecastDistance() const; //Returns the max distance, in which we still consider a sensed robot.
	FVector GetCollisionFreeVelocity(const FVector & PreferedVelocity) const;

	FTimerHandle MyTimeHandle;

	//Casted controller for quicker access
	ARobotAIController* AIController;

	bool bIsDebug = false;

	bool TryLink(int32 LinkIndex, const TWeakObjectPtr<ARobotPawn> Robot);
	TMap<TWeakObjectPtr<ARobotPawn>, float> IgnoreForLink; // Map stores for every ignored robot the time we started to ignore him. This value is used to determine if we still ignore him or remove.

	//States shared with other robots
	FVector2D GoalLocation = FVector2D(-1.0f, -1.0f);
	FVector HolonomicVelocity = FVector::ZeroVector;
	EStatusIndicator::Status MyStatusIndicator = EStatusIndicator::Moving;
	TArray<TWeakObjectPtr<ARobotPawn>> NeighborLinks; //0 UpLink, 1 LeftLink, 2 DownLink, 3 RightLink
	bool bLeftIsFree = false; bool bRightIsFree = false; bool bTopIsFree = false;

	int32 OldRow = 0;
	

	int32 MovementStrategy = 0; // 0 Indifferent, -1 go left, 1 go right;
	float ContentChangeSince = 0.0f; // Used to delay changing Content_FullRow status to something different.
	float UnsureChangeSince = 0.0f;
	float StrategyChangeSince = 0.0f;
	float LeftSpaceChangeSince = 0.0f;
	float RightSpaceChangeSince = 0.0f;

	void DisplayLinksAndStates(bool bShowStates = true, bool bShowLinks = true) const;
	void UpdateMyStatusIndicator();
	//void GetForcastInformation(TArray<FVector> &RobotLocations, TArray<FVector> & FormationLocations);
	//void GatherEnvironmentInfo(TArray<FVector> & RobotLocations, TArray<FVector> & FormationLocations);
	
	float ChangeContentFullRowDelay = 2.8f;
	float ChangeContentUnsureDelay = 1.2f; //1.2f;
	float StrategyChangeDelay = 0.1f; // 0.1f
	float SpaceChangeDelay = 0.2f; // 0.2f

	float RangeExtensionFactor = 1.36f; // 1.36 = 1.9 
	float ImproveLocationFactor = 0.9f; // 1.0f
	float ForecastFactor = 1.4;
};
