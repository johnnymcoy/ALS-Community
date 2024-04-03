// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CustomAnimInstance.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCustomAnimInstance : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ALSV4_CPP_API ICustomAnimInstance
{
	GENERATED_BODY()

public:

	/** For Setting the characters blind fire	*/
	virtual void SetFiringWeapon(const bool bValue) = 0;

	/** Setting The Location and Rotation of the Recoil	*/
	virtual void SetRecoilTransform(const FTransform& Transform) = 0;
	/** Setting the Pivot Point for the Gun to Rotate Around when Recoiling*/
	virtual void SetPivotPoint(const FTransform& Transform) = 0;

	/** For When the feet get stuck and need disabling	*/
	virtual void DisableFootIK(const float DelayTime) = 0;

	
};
