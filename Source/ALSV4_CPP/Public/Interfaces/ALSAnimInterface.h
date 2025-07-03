// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Library/ALSAnimationStructLibrary.h"
#include "UObject/Interface.h"
#include "ALSAnimInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UALSAnimInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ALSV4_CPP_API IALSAnimInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	
	// UFUNCTION(BlueprintCallable, Category="ALS|Essential Information")
	virtual void SetEssentialInfo(const FALSAnimValues& Value) = 0;
	
	virtual void SetOptimize(const bool bValue) = 0;
	
	virtual void SetMovementState(const FALSMovementState& Value) = 0;
	virtual void SetOverlayOverrideState(const int32 Value) = 0;
	virtual void SetGait(const FALSGait& Value) = 0;
	virtual void SetStance(const FALSStance& Value) = 0;
	virtual void SetViewMode(const EALSViewMode Value) = 0;
	virtual void SetOverlayState(const FALSOverlayState& Value) = 0;
	virtual void SetNewGroundedEntryState(const FALSGroundedEntryState& Value) = 0;
	virtual void SetRotationMode(const FALSRotationMode Value) = 0;
	

};
