// UnitControl © 2019 Niklaus Houska. All Rights Reserved 

#pragma once

#include "GameFramework/HUD.h"
#include "Info_HUD.generated.h"

/**
 * 
 */
UCLASS()
class UNITCONTROL_API AInfo_HUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void DrawHUD() override; // same as tick

	FVector2D InitialMousePoint;

	FVector2D CurrentMousePoint;

	bool bStartMoving = false;

	FVector2D GetMousePos2D();

	TArray<FVector> PositionToDraw;
	
};
