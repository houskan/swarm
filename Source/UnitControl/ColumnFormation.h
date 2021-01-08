// UnitControl © 2019 Niklaus Houska. All Rights Reserved 

#pragma once

#include "Formation.h"
#include "ColumnFormation.generated.h"

/**
 * 
 */
UCLASS()
class UNITCONTROL_API UColumnFormation : public UFormation
{
	GENERATED_BODY()
	
public:

	void SetColumnCount(int32 NewCount);

	virtual int32 GetColumnCount(int32 Row = 0) const override;

	void SetIsWedge(bool inBool);

	bool GetIsWedge() const;
	
protected:

	int32 ColumnCount = 10;

	bool bIsWedge = false;
};
