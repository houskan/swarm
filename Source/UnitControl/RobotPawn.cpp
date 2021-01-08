// UnitControl © 2019 Niklaus Houska. All Rights Reserved

#include "UnitControl.h"
#include "RobotAIController.h"
#include "RobotMovementComponent.h"
#include "RobotSensingComponent.h"
#include "Formation.h"
#include "RobotPawn.h"
#include "CollisionAvoidance.h"


// Sets default values
ARobotPawn::ARobotPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AutoPossessPlayer = EAutoReceiveInput::Disabled;
	AIControllerClass = ARobotAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	UCapsuleComponent* CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Root Component"));

	RootComponent = CapsuleComp;
	CapsuleComp->InitCapsuleSize(30.5f, 41.5f);
	CapsuleComp->SetCollisionProfileName(TEXT("Swarm Robot"));
	CapsuleComp->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Component"));
	StaticMeshComp->SetupAttachment(RootComponent);

	SensingComp = CreateDefaultSubobject<URobotSensingComponent>(TEXT("Sensing Component"));
	SensingComp->OnSeePawn.AddDynamic(this, &ARobotPawn::OnPawnSeen);

	MovementComp = CreateDefaultSubobject<URobotMovementComponent>(TEXT("Movement Component"));
	MovementComp->SetUpdatedComponent(RootComponent);


	static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT("StaticMesh'/Game/RobotModel.RobotModel'"));
	UStaticMesh* Asset = MeshAsset.Object;
	StaticMeshComp->SetStaticMesh(Asset);
}

// Called when the game starts or when spawned
void ARobotPawn::BeginPlay()
{
	Super::BeginPlay();

	NeighborLinks.AddDefaulted(4);

	AIController = Cast<ARobotAIController>(GetController());
	if (!AIController)
	{
		UE_LOG(LogTemp, Warning, TEXT("AIController cast failed at BeginPlay()"));
	}
	CollisionAvoidanceComp = NewObject<UCollisionAvoidance>(this, UCollisionAvoidance::StaticClass());
	CollisionAvoidanceComp->PrecomputePAHV();

	GetWorld()->GetTimerManager().SetTimer(MyTimeHandle, this, &ARobotPawn::UpdateGoalLocation, 0.1f, true, 0.05f);
}

void ARobotPawn::EndPlay(const EEndPlayReason::Type EEndPlayReason)
{
	Super::EndPlay(EEndPlayReason);
}

// Called every frame
void ARobotPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!AIController)
		return;

	UFormation* Formation = AIController->GetFormation();
	if (!Formation)
		return;

	FVector MyLocation = GetActorLocation(); MyLocation.Z = 0.0f;
	FVector MyGoal = Formation->GetWorldLocation(GoalLocation);
	FVector2D MyClosestFormationPlace = Formation->GetClosestLocation(MyLocation);
	FVector2D MyFormationLocation = Formation->GetFormationLocation(MyLocation);
	const bool bIsMoveToFormation = MyFormationLocation.Y < -1.4f || MyFormationLocation.Y > Formation->GetColumnCount() + 0.4f || MyFormationLocation.X < -1.4f;
	//bool bIsMoveToFormation = false;
	bool bIsValidGoal = Formation->IsAValidFormationLocation(MyGoal, 10.0f);

	if (bIsMoveToFormation && !bIsValidGoal)
	{
		MyGoal = Formation->GetWorldLocation(MyClosestFormationPlace);
		GoalLocation = FVector2D(-1000.0f, -1000.0f); //Set to an invalid GoalLocation, so it doesn't interfere with decision making of robots inside the formation
	}
	else if (!bIsValidGoal)
	{
		GoalLocation = MyClosestFormationPlace;
		MyGoal = Formation->GetWorldLocation(MyClosestFormationPlace);
	}

	//UpdateMyStatusIndicator();
	DisplayLinksAndStates(false, false);

	FVector Direction = MyGoal - MyLocation;
	float EaseOutFactor = FMath::Min(1.0f, Direction.Size() / 20.0f);
	Direction.Normalize(); Direction.Z = 0.0f;
	Direction = 100.0f * EaseOutFactor * Direction;

	if (bLeftIsFree && bRightIsFree)
	{
		;//DrawDebugSphere(GetWorld(), GetActorLocation(), 30.0f, 4, FColor::Yellow, false, 0.05);
	}
	else if (bLeftIsFree)
	{
		;//DrawDebugSphere(GetWorld(), GetActorLocation(), 30.0f, 4, FColor::Orange, false, 0.05);
	}
	else if (bRightIsFree)
	{
		;//DrawDebugSphere(GetWorld(), GetActorLocation(), 30.0f, 4, FColor::Red, false, 0.05);
	}
	
	HolonomicVelocity = GetCollisionFreeVelocity(Direction);

	MovementComp->AddInputVector(HolonomicVelocity);
}

// Called to bind functionality to input
void ARobotPawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

}

void ARobotPawn::OnPawnSeen(APawn * SeenPawn) {}

void ARobotPawn::PrintDebugInfo()
{
	bIsDebug = !bIsDebug;
	MovementComp->bIsDebug = bIsDebug;
}

void ARobotPawn::SetGoalLocation(FVector2D InGoalLocation)
{
	GoalLocation = InGoalLocation;
}

void ARobotPawn::UpdateGoalLocation()
{
	const UFormation* Formation = AIController->GetFormation();
	if (!Formation) return;

	//Constant variables for quick access.
	const FVector MyLocation = GetActorLocation();
	const FVector2D MyFormationLocation = Formation->GetFormationLocation(MyLocation);
	const FVector2D MyClosestFormationPlace = Formation->GetClosestLocation(MyLocation);
	const float ForecastDistance = GetForecastDistance();
	const bool bImTopRow = MyClosestFormationPlace.X == 0.0f;
	const bool bImLeftMost = MyClosestFormationPlace.Y == 0.0f;
	const bool bImRightMost = FMath::Abs(MyClosestFormationPlace.Y - (Formation->GetColumnCount() - 1.0f)) < 0.01f;
	const TArray<TWeakObjectPtr<ARobotPawn>> SensedRobots = SensingComp->GetSensedRobots();
	const bool bIsMiddleRow = ((int32)MyClosestFormationPlace.Y) == (Formation->GetColumnCount() - 1) / 2;
	const bool bIsLeftHalf = ((int32)MyClosestFormationPlace.Y) < (Formation->GetColumnCount() - 1) / 2;
	const bool bIsRightHalf = ((int32)MyClosestFormationPlace.Y) > (Formation->GetColumnCount() - 1) / 2;

	if (FVector2D::Distance(MyClosestFormationPlace, MyFormationLocation) > ForecastDistance / Formation->GetSpacing())
		return; //Method only applies if we are close to the formation
	
	//Local variables
	TArray<TWeakObjectPtr<ARobotPawn>> ConsideredRobots;
	TArray<TWeakObjectPtr<ARobotPawn>> DominantRobots;
	FVector2D DesiredFormationLocation = GoalLocation;
	bool bHavePotentialTopLink = false; bool bHavePotentialLeftLink = false; bool bHavePotentialRightLink = false;

	//Clean all links that are outdated:
	for (int32 I = 0; I < 4; I++)
	{
		if (!NeighborLinks[I].IsValid())
			continue;
		if (FVector::Dist(MyLocation, NeighborLinks[I]->GetActorLocation()) > ForecastDistance)
			NeighborLinks[I] = nullptr;
		else if ((I == 0 || I == 2) && GoalLocation.X != NeighborLinks[I]->GetGoalLocation().X)
			NeighborLinks[I] = nullptr;
		else if ((I == 1 || I == 3) && GoalLocation.Y != NeighborLinks[I]->GetGoalLocation().Y)
			NeighborLinks[I] = nullptr;
	}

	bool bLeftFreeNew = !bImLeftMost;
	bool bRightFreeNew = !bImRightMost;

	//Filter list of sensed robots:
	for (TWeakObjectPtr<ARobotPawn> Robot : SensedRobots)
	{
		const float Distance = FVector::Dist(Robot->GetActorLocation(), MyLocation);
		if (Distance > ForecastDistance * RangeExtensionFactor)
			continue;
		ConsideredRobots.Add(Robot);
	}

	//Make links and remember dominant robots
	for (TWeakObjectPtr<ARobotPawn> Robot : ConsideredRobots)
	{
		const FVector2D RobotFormationLocation = Formation->GetFormationLocation(Robot->GetActorLocation());
		const FVector2D DirectionToRobot = RobotFormationLocation - MyFormationLocation;
		const float RobotYaw = FMath::Atan2(DirectionToRobot.Y, DirectionToRobot.X) * (180.0f / PI);

		int32 LinkIndex;
		bool bIsAbove = false; bool bIsLeft = false;
		if (RobotYaw < -135.0f || RobotYaw > 135.0f) //Up 
			LinkIndex = 0;
		else if (RobotYaw < -45.0f) // left
			LinkIndex = 1;
		else if (RobotYaw > 45.0f) // right
			LinkIndex = 3;
		else // Down
			LinkIndex = 2;

		if (RobotYaw < -90.0f || RobotYaw > 90.0f)
			bIsAbove = true;
		if (RobotYaw < 0.0f)
			bIsLeft = true;

		bool bIsDominant = (LinkIndex == 0) || (!(LinkIndex == 2) && bIsMiddleRow) || (bIsLeftHalf && LinkIndex == 1) || (bIsRightHalf && LinkIndex == 3);
	
		FVector2D FormationCenter = bIsDominant ? Robot->GetGoalLocation() : GoalLocation;
		FVector2D LinkPositionVertical = FormationCenter + FVector2D(1.0f, 0.0f);
		FVector2D LinkPositionHorizontal = (!bIsDominant && LinkIndex == 1) || (bIsDominant && LinkIndex == 3) ? FormationCenter + FVector2D(0.0f, -1.0f) : FormationCenter + FVector2D(0.0f, 1.0f);
		if ((LinkIndex == 1 || LinkIndex == 3) && !(Formation->IsAValidFormationLocation(Formation->GetWorldLocation(LinkPositionHorizontal))))
		{
			LinkIndex = bIsAbove ? 0 : 2;
			bIsDominant = LinkIndex == 0;
			FormationCenter = bIsDominant ? Robot->GetGoalLocation() : GoalLocation;
			LinkPositionVertical = FormationCenter + FVector2D(1.0f, 0.0f);
		}

		if (bIsDominant)
			DominantRobots.Add(Robot);

		//Read info about free spaces from robots on same row.
		bool bIsSameRow = FMath::Abs(MyClosestFormationPlace.X - Formation->GetClosestLocation(Robot->GetActorLocation()).X) < 0.1f;
		if ((FVector::Dist(Robot->GetActorLocation(), MyLocation) < ForecastDistance || Robot->GetStatusIndicator() == EStatusIndicator::Moving) && bIsSameRow)
		{
			if (bIsLeft)
				bLeftFreeNew = bLeftFreeNew && Robot->IsLeftFree();
			else
				bRightFreeNew = bRightFreeNew && Robot->IsRightFree();
		}

		//Continue to next robot if link is in a different row or column
		float RowOfLinkRobot = bIsDominant ? MyClosestFormationPlace.X : Formation->GetClosestLocation(Robot->GetActorLocation()).X;
		if (LinkIndex == 0 && FMath::Abs(LinkPositionVertical.Y - MyClosestFormationPlace.Y) > 0.1f || (LinkIndex == 1 || LinkIndex == 3) && FMath::Abs(LinkPositionHorizontal.X - RowOfLinkRobot) > 0.1f)
		{
			if (NeighborLinks[LinkIndex].IsValid() && NeighborLinks[LinkIndex].Get() == Robot.Get())
				NeighborLinks[LinkIndex] = nullptr;
			continue;
		}

		//We only do actual links with robots that are inside the forecast distance
		if (FVector::Dist(Robot->GetActorLocation(), MyLocation) > ForecastDistance)
			continue;

		bHavePotentialTopLink = bHavePotentialTopLink || LinkIndex == 0;
		bHavePotentialLeftLink = bHavePotentialLeftLink || LinkIndex == 1;
		bHavePotentialRightLink = bHavePotentialRightLink || LinkIndex == 3;
	
		if (IgnoreForLink.Contains(Robot))
		{
			float IgnoreSince = IgnoreForLink[Robot];
			if (GetWorld()->GetTimeSeconds() - IgnoreSince > -0.5f)
				IgnoreForLink.FindAndRemoveChecked(Robot);
			else
				continue;
		}

		if (TryLink(LinkIndex, Robot))
			NeighborLinks[LinkIndex] = Robot;
	}
	UpdateMyStatusIndicator();

	bTopIsFree = !bImTopRow && !bHavePotentialTopLink;
	if (NeighborLinks[0].IsValid())
		bTopIsFree = NeighborLinks[0]->IsTopFree();
	

	if (!bLeftIsFree && bLeftFreeNew && (OldRow == (int32)(MyClosestFormationPlace.X + 0.1f)))
	{
		if (LeftSpaceChangeSince == 0.0f)
		{
			LeftSpaceChangeSince = GetWorld()->GetTimeSeconds();
		}
		else if (GetWorld()->GetTimeSeconds() - LeftSpaceChangeSince > SpaceChangeDelay)
		{
			LeftSpaceChangeSince = 0.0f;
			bLeftIsFree = bLeftFreeNew;
		}
	}
	else
	{
		LeftSpaceChangeSince = 0.0f;
		bLeftIsFree = bLeftFreeNew;
	}

	if (!bRightIsFree && bRightFreeNew && (OldRow == (int32)(MyClosestFormationPlace.X + 0.1f)))
	{
		if (RightSpaceChangeSince == 0.0f)
		{
			RightSpaceChangeSince = GetWorld()->GetTimeSeconds();
		}
		else if (GetWorld()->GetTimeSeconds() - RightSpaceChangeSince > SpaceChangeDelay)
		{
			RightSpaceChangeSince = 0.0f;
			bRightIsFree = bRightFreeNew;
		}
	}
	else
	{
		RightSpaceChangeSince = 0.0f;
		bRightIsFree = bRightFreeNew;
	}

	FVector2D ImprovedVertical;
	FVector2D ImprovedHorizontal;
	bool bImproveVertical = GetImprovedLocation(ImprovedVertical, ImprovedHorizontal, bHavePotentialLeftLink, bHavePotentialRightLink);

	if (bImproveVertical && !IsLocationConflicted(ImprovedVertical, DominantRobots))
		DesiredFormationLocation = ImprovedVertical;
	else if (ImprovedHorizontal.Y > -9.0f && !IsLocationConflicted(ImprovedHorizontal, DominantRobots))
		DesiredFormationLocation = ImprovedHorizontal;
	else if (!IsLocationConflicted(GoalLocation, DominantRobots, true))
		DesiredFormationLocation = GoalLocation;
	else if (!IsLocationConflicted(MyClosestFormationPlace, DominantRobots, true))
		DesiredFormationLocation = MyClosestFormationPlace;
	else
		DesiredFormationLocation = GetBackUpLocation();

	OldRow = (int32)(MyClosestFormationPlace.X + 0.1f);
	GoalLocation = DesiredFormationLocation;
}

bool ARobotPawn::TryLink(int32 LinkIndex, const TWeakObjectPtr<ARobotPawn> Robot)
{
	int32 OtherIndex = (LinkIndex + 2) % 4;
	bool bIsApproved = false;
	const FVector MyLocation = GetActorLocation();
	const FVector OtherLocation = Robot->GetActorLocation();

	for (int32 I = 0; I < 4; I++)
	{
		if (I != LinkIndex && NeighborLinks[I].IsValid() && NeighborLinks[I].Get() == Robot.Get()) NeighborLinks[I].Reset();
	}
	
	TWeakObjectPtr<ARobotPawn> MyLink = NeighborLinks[LinkIndex];
	bool bLinkValid = MyLink.IsValid();

	TWeakObjectPtr<ARobotPawn> OthersLink = Robot->GetLink(OtherIndex);
	bool bOthersValid = OthersLink.IsValid();

	int32 DebugCode = -1;
	//Both links are null
	if (!bLinkValid && !bOthersValid)
	{
		DebugCode = 0;
		bIsApproved = true;
	}
	//Own is null, other has one
	else if (!bLinkValid & bOthersValid)
	{
		DebugCode = 1;
		//if other's link is us, we are good to go.
		if (OthersLink.Get() == this) {
			DebugCode = 11;
			bIsApproved = true;
		}
	}
	//We have a link, he has not
	else if (bLinkValid && !bOthersValid)
	{
		FVector Middle = FMath::Lerp(MyLocation, OtherLocation, 0.5f);
		DebugCode = 2;

		//if he is my previous link, i keep him.
		if (Robot.Get() == MyLink.Get())
		{
			DebugCode = 21;
			bIsApproved = true;
		}
		//If he is in between my previous link, break link and try with him. Ignore other for some time.
		else if (FVector::Dist(MyLocation, OtherLocation) < FVector::Dist(MyLocation, MyLink->GetActorLocation()))
		{
			DebugCode = 22;
			IgnoreForLink.Add(MyLink, GetWorld()->GetTimeSeconds());
			bIsApproved = true;
		}
		/*FVector Middle = FMath::Lerp(MyLocation, OtherLocation, 0.5f);
		DebugCode = 2;
		
		//if he is my previous link, i keep him.
		if (Robot.Get() == MyLink.Get())
		{
			DebugCode = 21;
			bIsApproved = true;
		}
		//If he is in between my previous link, break link and try with him. Ignore other for some time.
		else if (FVector::Dist(Middle, OtherLocation) < AIController->GetFormation()->GetSpacing() / 2.0f)
		{
			DebugCode = 22;
			IgnoreForLink.Add(MyLink, GetWorld()->GetTimeSeconds());
			bIsApproved = true;
		}*/
	}
	//Both have a valid link
	else if (bLinkValid && bOthersValid)
	{
		DebugCode = 3;
		//Our previous link was him
		if (MyLink.Get() == Robot.Get())
		{
			DebugCode = 31;
			//If other's link is, we are good to go
			if (OthersLink.Get() == this)
			{
				DebugCode = 311;
				bIsApproved = true;
			}
			//Other's link is not us, we remove the link.
			else
			{
				DebugCode = 312;
				IgnoreForLink.Add(Robot, GetWorld()->GetTimeSeconds());
				NeighborLinks[LinkIndex] = nullptr;
			}
		}
		//Our previous link is not him, try to break link.... 
		else
		{
			DebugCode = 32;
			//if he is closer then the previous, replace link
			if (FVector::Dist(MyLocation, OtherLocation) < FVector::Dist(MyLocation, MyLink->GetActorLocation()))
			{
				DebugCode = 321;
				IgnoreForLink.Add(MyLink, GetWorld()->GetTimeSeconds());
				bIsApproved = true;
			}
		}
	}
	int32 SuccessIndicator = bIsApproved ? 1 : 0;
	if (bIsDebug)
		UE_LOG(LogTemp, Warning, TEXT("TryLink path taken: %d, %d"), DebugCode, SuccessIndicator);

	return bIsApproved;
}

void ARobotPawn::DisplayLinksAndStates(bool bShowStates, bool bShowLinks) const
{
	const FVector MyLocation = GetActorLocation();
	if (bShowLinks)
	{
		if (NeighborLinks[0].IsValid())
		{
			DrawDebugLine(GetWorld(), MyLocation, NeighborLinks[0]->GetActorLocation() + FVector(0.0f, 0.0f, 20.0f), FColor::Purple, false, 0.05, 1, 3.0f);
		}
		if (NeighborLinks[1].IsValid())
		{
			DrawDebugLine(GetWorld(), MyLocation, NeighborLinks[1]->GetActorLocation() + FVector(0.0f, 0.0f, 20.0f), FColor::Green, false, 0.05, 1, 3.0f);
		}
		if (NeighborLinks[3].IsValid())
		{
			DrawDebugLine(GetWorld(), MyLocation, NeighborLinks[3]->GetActorLocation() + FVector(0.0f, 0.0f, 20.0f), FColor::Red, false, 0.05, 1, 3.0f);
		}
		if (NeighborLinks[2].IsValid())
		{
			DrawDebugLine(GetWorld(), MyLocation, NeighborLinks[2]->GetActorLocation() + FVector(0.0f, 0.0f, 20.0f), FColor::Yellow, false, 0.05, 1, 3.0f);
		}
	}

	if (bShowStates)
	{
		switch (MyStatusIndicator)
		{
		case EStatusIndicator::Content_FullRow:
			DrawDebugSphere(GetWorld(), MyLocation + FVector(0.0f, 0.0f, 20.0f), 15.0f, 3, FColor::Green, false, 0.05f);
			break;
		case EStatusIndicator::Content_SpaceOnBoth:
			DrawDebugSphere(GetWorld(), MyLocation + FVector(0.0f, 0.0f, 20.0f), 15.0f, 3, FColor::Yellow, false, 0.05f);
			break;
		case EStatusIndicator::Content_SpaceOnRight:
			DrawDebugSphere(GetWorld(), MyLocation + FVector(0.0f, 0.0f, 20.0f), 15.0f, 3, FColor::Red, false, 0.05f);
			break;
		case EStatusIndicator::Content_SpaceOnLeft:
			DrawDebugSphere(GetWorld(), MyLocation + FVector(0.0f, 0.0f, 20.0f), 15.0f, 3, FColor::Orange, false, 0.05f);
			break;
		case EStatusIndicator::Moving:
			DrawDebugSphere(GetWorld(), MyLocation + FVector(0.0f, 0.0f, 20.0f), 15.0f, 3, FColor::Blue, false, 0.05f);
			break;
		case EStatusIndicator::Content_UnsureBoth:
			DrawDebugSphere(GetWorld(), MyLocation + FVector(0.0f, 0.0f, 20.0f), 15.0f, 3, FColor::Purple, false, 0.05f);
			break;
		case EStatusIndicator::Content_UnsureLeft:
			DrawDebugSphere(GetWorld(), MyLocation + FVector(0.0f, 0.0f, 20.0f), 15.0f, 3, FColor::Black, false, 0.05f);
			break;
		case EStatusIndicator::Content_UnsureRight:
			DrawDebugSphere(GetWorld(), MyLocation + FVector(0.0f, 0.0f, 20.0f), 15.0f, 3, FColor::White, false, 0.05f);
			break;
		default:
			break;
		}
	}
}

void ARobotPawn::UpdateMyStatusIndicator()
{
	const FVector MyLocation = GetActorLocation();
	const FVector MyGoal = AIController->GetFormation()->GetWorldLocation(GoalLocation);
	FVector2D MyFormationLocation = AIController->GetFormation()->GetClosestLocation(MyGoal);
	bool bValidGoal = AIController->GetFormation()->IsAValidFormationLocation(MyGoal, 10.0f);
	EStatusIndicator::Status NewStatus = MyStatusIndicator;


	if (bValidGoal && FVector::DistSquaredXY(MyGoal, MyLocation) < 100.0f)
	{
		TWeakObjectPtr<ARobotPawn> LeftCashed = NeighborLinks[1];
		TWeakObjectPtr<ARobotPawn> RightCashed = NeighborLinks[3];

		bool bIsLeftFree = !(MyFormationLocation.Y == 0.0f);
		bool bIsRightFree = !(MyFormationLocation.Y == AIController->GetFormation()->GetColumnCount() - 1.0f);

		bool bIsLeftFreeUnsure = bIsLeftFree;
		bool bIsRightFreeUnsure = bIsRightFree;

		if (bIsLeftFree && LeftCashed.IsValid())
		{
			EStatusIndicator::Status LeftStatus = LeftCashed->GetStatusIndicator();
			bIsLeftFree = LeftStatus == EStatusIndicator::Content_SpaceOnLeft || LeftStatus == EStatusIndicator::Content_SpaceOnBoth;
			bIsLeftFreeUnsure = LeftStatus == EStatusIndicator::Content_UnsureLeft || LeftStatus == EStatusIndicator::Content_UnsureBoth || LeftStatus == EStatusIndicator::Moving;
		}
		if (bIsRightFree && RightCashed.IsValid())
		{
			EStatusIndicator::Status RightStatus = RightCashed->GetStatusIndicator();
			bIsRightFree = RightStatus == EStatusIndicator::Content_SpaceOnRight || RightStatus == EStatusIndicator::Content_SpaceOnBoth;
			bIsRightFreeUnsure = RightStatus == EStatusIndicator::Content_UnsureRight || RightStatus == EStatusIndicator::Content_UnsureBoth || RightStatus == EStatusIndicator::Moving;
		}

		
		if (bIsRightFree && bIsLeftFree)
			NewStatus = EStatusIndicator::Content_SpaceOnBoth;
		else if (bIsRightFree)
			NewStatus = EStatusIndicator::Content_SpaceOnRight;
		else if (bIsLeftFree)
			NewStatus = EStatusIndicator::Content_SpaceOnLeft;
		else if (bIsLeftFreeUnsure && bIsRightFreeUnsure)
			NewStatus = EStatusIndicator::Content_UnsureBoth;
		else if (bIsLeftFreeUnsure)
			NewStatus = EStatusIndicator::Content_UnsureLeft;
		else if (bIsRightFreeUnsure)
			NewStatus = EStatusIndicator::Content_UnsureRight;
		else if (!bIsLeftFree && !bIsRightFree && !bIsRightFreeUnsure && !bIsLeftFreeUnsure)
			NewStatus = EStatusIndicator::Content_FullRow;
	}
	else
	{
		NewStatus = EStatusIndicator::Moving;
	}

	bool bBecomeContentWithSpace = NewStatus == EStatusIndicator::Content_SpaceOnBoth || NewStatus == EStatusIndicator::Content_SpaceOnLeft || NewStatus == EStatusIndicator::Content_SpaceOnRight;
	bool bUnsure = MyStatusIndicator == EStatusIndicator::Content_UnsureLeft || MyStatusIndicator == EStatusIndicator::Content_UnsureRight || MyStatusIndicator == EStatusIndicator::Content_UnsureBoth;
	bool bBecomeUnsure = NewStatus == EStatusIndicator::Content_UnsureRight || NewStatus == EStatusIndicator::Content_UnsureLeft || NewStatus == EStatusIndicator::Content_UnsureBoth;

	bool bApproved = false;
	if (MyStatusIndicator == EStatusIndicator::Content_FullRow && NewStatus != MyStatusIndicator && NewStatus != EStatusIndicator::Moving)
	{
		if (ContentChangeSince == 0.0f)
		{
			ContentChangeSince = GetWorld()->GetTimeSeconds();
		}
		else if (GetWorld()->GetTimeSeconds() - ContentChangeSince > ChangeContentFullRowDelay)
		{
			bApproved = true;
			ContentChangeSince = 0.0f;
		}
	}
	else if ((bUnsure && bBecomeContentWithSpace && NewStatus != EStatusIndicator::Content_FullRow))
	{
		if (UnsureChangeSince == 0.0f)
			UnsureChangeSince = GetWorld()->GetTimeSeconds();
		else if (GetWorld()->GetTimeSeconds() - UnsureChangeSince > ChangeContentUnsureDelay)
		{
			bApproved = true;
			UnsureChangeSince = 0.0f;
		}
	}
	else
	{
		bApproved = true;
		ContentChangeSince = 0.0f;
	}

	if (bApproved)
	{
		MyStatusIndicator = NewStatus;
		if (!bUnsure && bBecomeUnsure)
		{
			UnsureChangeSince = GetWorld()->GetTimeSeconds();
		}
		else if (!bUnsure)
		{
			UnsureChangeSince = 0.0f;
		}
	}
}

bool ARobotPawn::GetImprovedLocation(FVector2D & ImprovedVertical, FVector2D & ImprovedHorizontal, bool bHavePotentialLeftLink, bool bHavePotentialRightLink)
{
	const FVector2D MyClosestFormationPlace = AIController->GetFormation()->GetClosestLocation(GetActorLocation());
	const bool bIsMiddleRow = ((int32)MyClosestFormationPlace.Y) == (AIController->GetFormation()->GetColumnCount() - 1) / 2;
	const bool bIsLeftHalf = ((int32)MyClosestFormationPlace.Y) < (AIController->GetFormation()->GetColumnCount() - 1) / 2;
	const bool bIsRightHalf = ((int32)MyClosestFormationPlace.Y) > (AIController->GetFormation()->GetColumnCount() - 1) / 2;
	const bool bMakeSpace = NeighborLinks[2].IsValid() && (NeighborLinks[2]->GetStatusIndicator() == EStatusIndicator::Content_SpaceOnLeft || NeighborLinks[2]->GetStatusIndicator() == EStatusIndicator::Content_SpaceOnBoth || NeighborLinks[2]->GetStatusIndicator() == EStatusIndicator::Content_SpaceOnRight);
	//const bool bMakeSpace = NeighborLinks[2].IsValid() && NeighborLinks[2]->GetStatusIndicator() != EStatusIndicator::Moving;
	ImprovedVertical = FVector2D(0.0f, -10.0f);
	ImprovedHorizontal = FVector2D(0.0f, -10.0f);
	
	
	
	if (FVector2D::Distance(GoalLocation, AIController->GetFormation()->GetFormationLocation(GetActorLocation())) < ImproveLocationFactor)
	{
		if (bTopIsFree)
		{
			ImprovedVertical = MyClosestFormationPlace + FVector2D(-1.0f, 0.0f);
		}
		if (MyStatusIndicator != EStatusIndicator::Content_FullRow)
		{
			FVector2D LeftLocation = MyClosestFormationPlace + FVector2D(0.0f, -1.0f);
			FVector2D RightLocation = MyClosestFormationPlace + FVector2D(0.0f, 1.0f);
			bool bGoLeft = bMakeSpace && bLeftIsFree;
			bool bGoRight = bMakeSpace && bRightIsFree;

			if (bIsLeftHalf || bIsMiddleRow)
				bGoLeft = bGoLeft || bLeftIsFree && bHavePotentialRightLink;
				
			if (bIsRightHalf || bIsMiddleRow)
				bGoRight = bGoRight || bRightIsFree && bHavePotentialLeftLink;
			
			if (MovementStrategy == 0)
			{
				if (bIsLeftHalf || bIsMiddleRow)
				{
					if (bGoLeft)
					{
						ImprovedHorizontal = LeftLocation;
						MovementStrategy = -1;
					}
					else if (bGoRight)
					{
						ImprovedHorizontal = RightLocation;
						MovementStrategy = 1;
					}
				}
				else
				{
					if (bGoRight)
					{
						ImprovedHorizontal = RightLocation;
						MovementStrategy = 1;
					}
					else if (bGoLeft)
					{
						ImprovedHorizontal = LeftLocation;
						MovementStrategy = -1;
					}
				}
			}
			else if (MovementStrategy == 1)
			{
				if (bGoRight)
				{
					StrategyChangeSince = 0.0f;
					ImprovedHorizontal = RightLocation;
				}
				else if (bGoLeft)
				{
					if (StrategyChangeSince == 0.0f && !(bLeftIsFree && bMakeSpace))
						StrategyChangeSince = GetWorld()->GetTimeSeconds();
					else if (GetWorld()->GetTimeSeconds() - StrategyChangeSince > StrategyChangeDelay || bLeftIsFree && bMakeSpace)
					{
						ImprovedHorizontal = LeftLocation;
						MovementStrategy = -1;
						StrategyChangeSince = 0.0f;
					}
				}
			}
			else if (MovementStrategy == -1)
			{
				if (bGoLeft)
				{
					StrategyChangeSince = 0.0f;
					ImprovedHorizontal = LeftLocation;
				}
				else if (bGoRight)
				{
					if (StrategyChangeSince == 0.0f && !(bRightIsFree && bMakeSpace))
						StrategyChangeSince = GetWorld()->GetTimeSeconds();
					else if (GetWorld()->GetTimeSeconds() - StrategyChangeSince > StrategyChangeDelay || bRightIsFree && bMakeSpace)
					{
						ImprovedHorizontal = RightLocation;
						MovementStrategy = 1;
						StrategyChangeSince = 0.0f;
					}
				}
			}
		}
	}
	return ImprovedVertical.Y > -9.0f;
}

FVector2D ARobotPawn::GetBackUpLocation()
{
	const FVector2D MyClosestFormationPlace = AIController->GetFormation()->GetClosestLocation(GetActorLocation());
	const bool bIsMiddleRow = ((int32)MyClosestFormationPlace.Y) == (AIController->GetFormation()->GetColumnCount() - 1) / 2;
	const bool bIsLeftHalf = ((int32)MyClosestFormationPlace.Y) < (AIController->GetFormation()->GetColumnCount() - 1) / 2;
	const bool bIsRightHalf = ((int32)MyClosestFormationPlace.Y) > (AIController->GetFormation()->GetColumnCount() - 1) / 2;

	FVector2D Result;
	bool bGoLeft = bLeftIsFree;
	bool bGoRight = bRightIsFree;
	if (!bGoLeft && !bGoRight)
		Result = MyClosestFormationPlace + FVector2D(1.0f, 0.0f);
	else
	{
		if (MovementStrategy == 0)
		{
			if (bIsLeftHalf || bIsMiddleRow)
			{
				if (bGoLeft)
				{
					Result = MyClosestFormationPlace + FVector2D(0.0f, -1.0f);
					MovementStrategy = -1;
				}
				else if (bGoRight)
				{
					Result = MyClosestFormationPlace + FVector2D(0.0f, 1.0f);
					MovementStrategy = 1;
				}
			}
			else
			{
				if (bGoRight)
				{
					Result = MyClosestFormationPlace + FVector2D(0.0f, 1.0f);
					MovementStrategy = 1;
				}
				else if (bGoLeft)
				{
					Result = MyClosestFormationPlace + FVector2D(0.0f, -1.0f);
					MovementStrategy = -1;
				}
			}
		}
		else if (MovementStrategy == 1)
		{
			if (bGoRight)
				Result = MyClosestFormationPlace + FVector2D(0.0f, 1.0f);
			else if (bGoLeft)
			{
				Result = MyClosestFormationPlace + FVector2D(0.0f, -1.0f);
				MovementStrategy = -1;
			}
		}
		else
		{
			if (bGoLeft)
				Result = MyClosestFormationPlace + FVector2D(0.0f, -1.0f);
			else if (bGoRight)
			{
				Result = MyClosestFormationPlace + FVector2D(0.0f, 1.0f);
				MovementStrategy = 1;
			}
		}
	}
	return Result;
}

bool ARobotPawn::IsLocationConflicted(const FVector2D & FormationLocation, const TArray<TWeakObjectPtr<ARobotPawn>>& DominantRobots, bool bCheckNeighborInvariants) const
{
	for (TWeakObjectPtr<ARobotPawn> Robot : DominantRobots)
	{
		if (FVector2D::Distance(Robot->GetGoalLocation(), FormationLocation) < 0.1f)
			return true;
	}
	if (bCheckNeighborInvariants)
	{
		const FVector2D MyClosestFormationPlace = AIController->GetFormation()->GetClosestLocation(GetActorLocation());
		const bool bIsLeftHalf = ((int32)MyClosestFormationPlace.Y) < (AIController->GetFormation()->GetColumnCount() - 1) / 2;
		const bool bIsRightHalf = ((int32)MyClosestFormationPlace.Y) > (AIController->GetFormation()->GetColumnCount() - 1) / 2;

		bool bNotConflicted = true;
		bNotConflicted = bNotConflicted && (!NeighborLinks[0].IsValid() || NeighborLinks[0]->GetGoalLocation().X <= FormationLocation.X);
		bNotConflicted = bNotConflicted && (bIsRightHalf || (!NeighborLinks[1].IsValid() || NeighborLinks[1]->GetGoalLocation().Y < FormationLocation.Y));
		bNotConflicted = bNotConflicted && (bIsLeftHalf || (!NeighborLinks[3].IsValid() || NeighborLinks[3]->GetGoalLocation().Y > FormationLocation.Y));
		return !bNotConflicted;
	}
	return false;
}

float ARobotPawn::GetForecastDistance() const
{
	UFormation* CurrentFormation = AIController->GetFormation();
	if (!CurrentFormation)
		return 0.0f;
	return CurrentFormation->GetSpacing() * ForecastFactor; //Assuming this value is less than the sight radius of the robot
}

FVector ARobotPawn::GetCollisionFreeVelocity(const FVector & PreferedVelocity) const
{
	if (!CollisionAvoidanceComp)
		return PreferedVelocity;

	const TArray<TWeakObjectPtr<ARobotPawn>> SensedRobots = SensingComp->GetSensedRobots();
	TArray<FVector> CollisionObstacles;

	for (TWeakObjectPtr<ARobotPawn> Robot : SensedRobots)
	{
		FVector RobotLocation = Robot->GetActorLocation();
		if (FVector::Dist(GetActorLocation(), RobotLocation) > GetForecastDistance() * 1.6f)
			continue;
		float MutualityFactor = Robot->GetStatusIndicator() == EStatusIndicator::Moving ? 0.5f : 0.5f;
		CollisionObstacles.Add(CollisionAvoidanceComp->CalculataCollisionObstacle(GetActorLocation(), RobotLocation, HolonomicVelocity, Robot->GetHolonomicVelocity(), MutualityFactor));
	}

	TArray<FVector2D> AllowedVelocities;
	if (!CollisionAvoidanceComp->CalculateAllowedVelocities(AllowedVelocities, GetActorRotation().Yaw, CollisionObstacles))
	{
		return FVector::ZeroVector;
	}
	return CollisionAvoidanceComp->GetOptimalHolonomicVelocity(AllowedVelocities, PreferedVelocity);
}
