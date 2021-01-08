// UnitControl © 2019 Niklaus Houska. All Rights Reserved 

#include "UnitControl.h"
#include "ColumnFormation.h"


void UColumnFormation::SetColumnCount(int32 NewCount)
{
	ColumnCount = NewCount;
}

int32 UColumnFormation::GetColumnCount(int32 Row) const
{
	return ColumnCount;
}

void UColumnFormation::SetIsWedge(bool inBool)
{
	bIsWedge = inBool;
}

bool UColumnFormation::GetIsWedge() const
{
	return bIsWedge;
}