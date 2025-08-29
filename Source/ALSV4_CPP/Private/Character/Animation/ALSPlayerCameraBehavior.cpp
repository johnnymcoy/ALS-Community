// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community


#include "Character/Animation/ALSPlayerCameraBehavior.h"


#include "Character/ALSBaseCharacter.h"

void UALSPlayerCameraBehavior::SetRotationMode(EALSRotationMode RotationMode)
{
	bVelocityDirection = RotationMode == EALSRotationMode::VelocityDirection;
	bLookingDirection = RotationMode == EALSRotationMode::LookingDirection;
	bAiming = RotationMode == EALSRotationMode::Aiming;
}

void UALSPlayerCameraBehavior::SetRightShoulder(const bool bInRightShoulder)
{
	bRightShoulder = bInRightShoulder;
}

void UALSPlayerCameraBehavior::SetAimDownSights(const bool bInAimDownSights)
{
	bAimDownSights = bInAimDownSights;
}

void UALSPlayerCameraBehavior::SetMovementAction(const EALSMovementAction InAction)
{
	MovementAction = InAction;
}

void UALSPlayerCameraBehavior::SetState(const EALSMovementState InState)
{
	MovementState = InState;
}

void UALSPlayerCameraBehavior::SetGait(const EALSGait InGait)
{
	Gait = InGait;
}

void UALSPlayerCameraBehavior::SetStance(const EALSStance InStance)
{
	Stance = InStance;
}

void UALSPlayerCameraBehavior::SetViewMode(const EALSViewMode InViewMode)
{
	ViewMode = InViewMode;
}

void UALSPlayerCameraBehavior::SetDebugView(const bool bInDebugView)
{
	bDebugView = bInDebugView;
}
