// UnitControl © 2019 Niklaus Houska. All Rights Reserved

#include "UnitControl.h"
#include "MasterController.h"
#include "Info_HUD.h"
#include "UnitControlGameModeBase.h"


AUnitControlGameModeBase::AUnitControlGameModeBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerControllerClass = AMasterController::StaticClass();
	HUDClass = AInfo_HUD::StaticClass();
	DefaultPawnClass = nullptr;
}
