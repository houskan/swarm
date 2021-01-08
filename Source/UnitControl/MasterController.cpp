// UnitControl © 2019 Niklaus Houska. All Rights Reserved 

#include "UnitControl.h"
#include "Info_HUD.h"
#include "RobotPawn.h"
#include "RobotAIController.h"
#include "Formation.h"
#include "ColumnFormation.h"
#include "MasterController.h"

AMasterController::AMasterController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AMasterController::Tick(float DeltaTime)
{
	
	if (HUDPtr == nullptr)
	{
		return;
	}

	if (HUDPtr->bStartMoving)
	{
		FHitResult RV_Hit;
		GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, RV_Hit);

		CurrentWorldLocation = RV_Hit.Location;

		FVector Orientation = InitialWorldLocation - CurrentWorldLocation;
		Orientation.Z = 0.0f;
		CurrentFormation->SetOrigin(FVector2D(InitialWorldLocation.X, InitialWorldLocation.Y));
		if (Orientation.Size() > 1.0f)
		{
			CurrentFormation->SetOrientation(FVector2D(-Orientation.X, -Orientation.Y));
		}
		UpdateFormation();
	}

	bool bIsDone = true;
	for (TWeakObjectPtr<ARobotPawn> Robot : ListOfRobots)
	{
		int32 ForbiddenRow = ListOfRobots.Num() / CurrentFormation->GetColumnCount();
		if (ListOfRobots.Num() % CurrentFormation->GetColumnCount() != 0)
			ForbiddenRow++;
		if (!(Robot->GetStatusIndicator() == EStatusIndicator::Content_FullRow || Robot->GetStatusIndicator() == EStatusIndicator::Content_SpaceOnBoth || Robot->GetStatusIndicator() == EStatusIndicator::Content_SpaceOnLeft || Robot->GetStatusIndicator() == EStatusIndicator::Content_SpaceOnRight))
		{
			bIsDone = false;
			break;
		}
		if (CurrentFormation->GetClosestLocation(Robot->GetActorLocation()).X - (float)ForbiddenRow > -0.5f)
		{
			bIsDone = false;
			break;
		}
	}
}

void AMasterController::BeginPlay()
{
	HUDPtr = Cast<AInfo_HUD>(GetHUD());
	CurrentFormation = NewObject<UFormation>(this, UColumnFormation::StaticClass());
	CurrentFormation->SetSpacing(140.0f);
	CurrentFormation->SetOrigin(FVector2D::ZeroVector);
	CurrentFormation->SetOrientation(FVector2D(-1.0f, 0.0f));
	UpdateFormation();
	GetWorld()->GetTimerManager().SetTimer(MyTimeHandle, this, &AMasterController::Initialize, 0.02f, false, 0.02f);
}

void AMasterController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("DeleteRobot", IE_Pressed, this, &AMasterController::DeleteRobotPressed);
	InputComponent->BindAction("SpawnRobot", IE_Pressed, this, &AMasterController::SpawnRobotPressed);

	InputComponent->BindAction("MoveFormation", IE_Pressed, this, &AMasterController::MoveFormationPressed);
	InputComponent->BindAction("MoveFormation", IE_Released, this, &AMasterController::MoveFormationReleased);

	InputComponent->BindAction("ToggleColumnCount", IE_Pressed, this, &AMasterController::ToggleColumnCount);
	InputComponent->BindAction("ToggleFormation", IE_Pressed, this, &AMasterController::ToggleFormation);
	InputComponent->BindAction("ToggleSpacing", IE_Pressed, this, &AMasterController::ToggleSpacing);

	InputComponent->BindAction("Debug", IE_Pressed, this, &AMasterController::Debug);
}


void AMasterController::SpawnRobotPressed()
{
	FHitResult RV_Hit;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, RV_Hit);

	//Quick check if we hit the ground or another Robot. 
	AStaticMeshActor* TestActor = Cast<AStaticMeshActor>(RV_Hit.GetActor());
	if (TestActor == nullptr) {
		return;
	}

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	FRotator RRot = FRotator(0.0f, FMath::FRand() * 360.0f, 0.0f);
	FVector SpawnLocation = FVector(RV_Hit.Location);

	ARobotPawn* SpawnedRobot = GetWorld()->SpawnActor<ARobotPawn>(SpawnLocation, RRot, SpawnInfo);
	if (SpawnedRobot != nullptr)
	{
		ARobotAIController* RobotController = Cast<ARobotAIController>(SpawnedRobot->GetController());
		RobotController->SetCurrentFormation(CurrentFormation);
		ListOfRobots.AddUnique(SpawnedRobot);
	}
}

void AMasterController::DeleteRobotPressed()
{
	FHitResult RV_Hit;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, RV_Hit);

	ARobotPawn* TestActor = Cast<ARobotPawn>(RV_Hit.GetActor());
	if (!TestActor || !TestActor->IsValidLowLevel()) {
		return;
	}

	ListOfRobots.Remove(TestActor);

	TestActor->K2_DestroyActor();
	TestActor = NULL;

	GetWorld()->ForceGarbageCollection(true);
}

void AMasterController::ToggleColumnCount()
{
	UColumnFormation* ColumnFormation = Cast<UColumnFormation>(CurrentFormation);
	if (ColumnFormation != nullptr)
	{
		int32 CurrentColumnCount = ColumnFormation->GetColumnCount();
		int32 NewColumnCount;

		if (CurrentColumnCount == 1)
		{
			NewColumnCount = 2;
		} 
		else if (CurrentColumnCount == 2)
		{
			NewColumnCount = 5;
		}
		else if (CurrentColumnCount == 5)
		{
			NewColumnCount = 10;
		} 
		else if (CurrentColumnCount == 10)
		{
			NewColumnCount = 3;
		} 
		else if (CurrentColumnCount == 3)
		{
			NewColumnCount = 1;
		}
		else
		{
			NewColumnCount = 1;
		}
		ColumnFormation->SetColumnCount(NewColumnCount);
		UpdateFormation();
	}
}


void AMasterController::ToggleFormation()
{
	UColumnFormation* ColumnFormation = Cast<UColumnFormation>(CurrentFormation);
	if (ColumnFormation)
	{
		ColumnFormation->SetIsWedge(!ColumnFormation->GetIsWedge());
		UpdateFormation();
	}

}

void AMasterController::ToggleSpacing()
{
	float CurrentSpacing = CurrentFormation->GetSpacing();
	float NewSpacing;
	if (FMath::Abs(CurrentSpacing - 70.0f) < 0.001f)
	{
		NewSpacing = 100.0f;
	}
	else if (FMath::Abs(CurrentSpacing - 100.0f) < 0.001f)
	{
		NewSpacing = 140.0f;
	}
	else if (FMath::Abs(CurrentSpacing - 140.0f) < 0.001f)
	{
		NewSpacing = 280.0f;
	}
	else
	{
		NewSpacing = 100.0f;
	}

	CurrentFormation->SetSpacing(NewSpacing);
	UpdateFormation();
}

void AMasterController::Debug()
{
	FHitResult RV_Hit;
	GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, RV_Hit);

	ARobotPawn* TestActor = Cast<ARobotPawn>(RV_Hit.GetActor());
	if (!TestActor || !TestActor->IsValidLowLevel()) {
		return;
	}
	TestActor->PrintDebugInfo();
}

void AMasterController::MoveFormationPressed()
{
	if (HUDPtr != nullptr)
	{
		FHitResult RV_Hit;
		GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, RV_Hit);
		InitialWorldLocation = RV_Hit.Location;
		HUDPtr->InitialMousePoint = HUDPtr->GetMousePos2D();
		HUDPtr->bStartMoving = true;
	}

}

void AMasterController::MoveFormationReleased()
{
	if (HUDPtr != nullptr)
		HUDPtr->bStartMoving = false;
}


void AMasterController::UpdateFormation()
{
	CurrentFormation->Precompute();
	if (HUDPtr)
		HUDPtr->PositionToDraw = CurrentFormation->GetFirstXRows(0);
}

void AMasterController::Initialize()
{
	for (TActorIterator<ARobotPawn> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		ARobotAIController* RobotController = Cast<ARobotAIController>((*ActorItr)->GetController());
		if (!RobotController)
			continue;
		if (!RobotController->GetFormation())
		{
			RobotController->SetCurrentFormation(CurrentFormation);
			ListOfRobots.AddUnique(*ActorItr);
		}
	}
	
	int32 Num = ListOfRobots.Num();
	int32 ColCount = CurrentFormation->GetColumnCount();
	int32 NumOfRows = Num / ColCount;
	NumOfRows = Num % ColCount != 0 ? NumOfRows + 1: NumOfRows;
	TArray<FVector> FinalLocations = CurrentFormation->GetFirstXRows(NumOfRows);
	bMapStarted = true;
}
