// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ALSCameraBehaviorInterface.generated.h"

enum class EALSMovementState : uint8;
enum class EALSRotationMode : uint8;
enum class EALSMovementAction : uint8;
enum class EALSGait : uint8;
enum class EALSStance : uint8;
enum class EALSViewMode : uint8;

// This class does not need to be modified.
UINTERFACE()
class UALSCameraBehaviorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ALSV4_CPP_API IALSCameraBehaviorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void SetRotationMode(EALSRotationMode RotationMode) = 0;
	// virtual void SetCameraBehavior(const EALSMovementState State, const EALSMovementAction Action, const bool bInRightShoulder, const EALSGait InGait,
	// 	const EALSStance InStance, const EALSViewMode InViewMode, const bool bInAimDownSights) = 0;

	virtual void SetRightShoulder(const bool bInRightShoulder) = 0;
	virtual void SetAimDownSights(const bool bInAimDownSights) = 0;
	virtual void SetMovementAction(const EALSMovementAction InAction) = 0;
	virtual void SetState(const EALSMovementState InState) = 0;
	virtual void SetGait(const EALSGait InGait) = 0;
	virtual void SetStance(const EALSStance InStance) = 0;
	virtual void SetViewMode(const EALSViewMode InViewMode) = 0;

	virtual void SetDebugView(const bool bInDebugView) = 0;


};
