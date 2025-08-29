// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ALSCharacterInput.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UALSCharacterInput : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ALSV4_CPP_API IALSCharacterInput
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	virtual void SprintAction(const bool bValue) = 0;
	virtual void CameraTapAction() = 0;
	virtual void CameraHeldAction() = 0;
	virtual void StanceAction() = 0;
	virtual void WalkAction() = 0;
	virtual void RagdollAction() = 0;
	virtual void VelocityDirectionAction() = 0;
	virtual void LookingDirectionAction() = 0;


};
