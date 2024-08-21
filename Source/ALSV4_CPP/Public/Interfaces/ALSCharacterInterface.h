// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ALSCharacterMovementInterface.h"
#include "ALSGravityMovementInterface.h"
#include "Library/ALSExtraData.h"
#include "UObject/Interface.h"
#include "ALSCharacterInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UALSCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ALSV4_CPP_API IALSCharacterInterface
{
	GENERATED_BODY()

public:

	// virtual void Ragdoll() = 0;
	// virtual USkeletalMeshComponent* GetCharacterMesh() = 0;
	// virtual UPrimitiveComponent* GetPrimitiveComponent() = 0;
	// virtual IALSCharacterMovementInterface* GetALSCharacterMovementInterface() const = 0;
	virtual IALSGravityMovementInterface* GetALSGravityMovementInterface() const = 0;
	
	// virtual void GravityDirectionChanged(EGravityDirectionMode OldGravityDirectionMode, EGravityDirectionMode CurrentGravityDirectionMode) = 0;


};
