// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ALSCameraInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UALSCameraInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ALSV4_CPP_API IALSCameraInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void Possess(APawn* Pawn) = 0;
	// virtual void SetRightShoulder(const bool bValue) = 0;
	virtual void RequestDrawDebugTargets(const FVector& PivotTargetLocation) = 0;

};
