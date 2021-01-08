// UnitControl © 2019 Niklaus Houska. All Rights Reserved 

#include "UnitControl.h"
#include "Info_HUD.h"


void AInfo_HUD::DrawHUD()
{
	if (bStartMoving)
	{
		CurrentMousePoint = GetMousePos2D();
		DrawLine(InitialMousePoint.X, InitialMousePoint.Y, CurrentMousePoint.X, CurrentMousePoint.Y, FLinearColor::Yellow, 2.0f);
		
	}
	for (FVector Pos : PositionToDraw)
	{
		FVector2D ScreenPos;
		GetOwningPlayerController()->ProjectWorldLocationToScreen(Pos, ScreenPos);
		DrawLine(ScreenPos.X, ScreenPos.Y, ScreenPos.X + 2.0f, ScreenPos.Y, FLinearColor::Yellow, 2.0f);
	}
}

FVector2D AInfo_HUD::GetMousePos2D()
{
	float PosX;
	float PosY;

	GetOwningPlayerController()->GetMousePosition(PosX, PosY);
	return FVector2D(PosX, PosY);
}
