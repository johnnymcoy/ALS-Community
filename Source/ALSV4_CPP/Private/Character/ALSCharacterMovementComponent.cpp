// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community


#include "Character/ALSCharacterMovementComponent.h"
#include "Character/ALSBaseCharacter.h"
#include "Components/SplineComponent.h"

#include "Curves/CurveVector.h"
#include "Library/ALSExtraData.h"

DECLARE_CYCLE_STAT(TEXT("ALS Movement Comp (All Functions)"), STATGROUP_ALS_Movement, STATGROUP_ALS);
DECLARE_CYCLE_STAT(TEXT("ALS Movement Comp (All Gravity Funcs)"), STATGROUP_ALS_Movement_Gravity, STATGROUP_ALS);


UALSCharacterMovementComponent::UALSCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NavAgentProps.bCanCrouch = true;

	// NetworkSmoothingMode = ENetworkSmoothingMode::Disabled;
	// RotationRate = FRotator(360.0f, 360.0f, 360.0f);

	// bAlignComponentToFloor = false;
	// bAlignComponentToGravity = false;
	bAlignGravityToBase = false;
	// bAlwaysRotateAroundCenter = false;
	// bApplyingNetworkMovementMode = false;
	bDirtyGravityDirection = false;
	bDisableGravityReplication = false;
	// bForceSimulateMovement = false;
	// bIgnoreOtherGravityScales = false;
	// bLandOnAnySurface = false;
	// bRevertToDefaultGravity = false;
	// bRotateVelocityOnGround = false;
	// bTriggerUnwalkableHits = false;
	GravityActor = nullptr;
	GravityDirectionMode = EGravityDirectionMode::Fixed;
	GravityVectorA = FVector::DownVector;
	GravityVectorB = FVector::ZeroVector;
	// LastUnwalkableHitTime = -1.0f;
	OldGravityScale = GravityScale;
	// SetThresholdParallelAngle(1.0f);

}

void UALSCharacterMovementComponent::OnMovementUpdated(float DeltaTime, const FVector& OldLocation,
                                                       const FVector& OldVelocity)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterMovementComponent::OnMovementUpdated);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);

	Super::OnMovementUpdated(DeltaTime, OldLocation, OldVelocity);

	
	if (!CharacterOwner)
	{
		return;
	}

	// Set Movement Settings
	if (bRequestMovementSettingsChange)
	{
		const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
		MaxWalkSpeed = UpdateMaxWalkSpeed;
		MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;

		bRequestMovementSettingsChange = false;
	}

	
	//~ Gravity Functions	//
	UpdateGravity();
	if(GetShouldReplicateGravity())
	{
		ReplicateGravityToClients();
	}

}

void UALSCharacterMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterMovementComponent::PhysWalking);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);

	if (CurrentMovementSettings.MovementCurve)
	{
		// Update the Ground Friction using the Movement Curve.
		// This allows for fine control over movement behavior at each speed.
		GroundFriction = CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Z;
	}
	Super::PhysWalking(deltaTime, Iterations);
}

float UALSCharacterMovementComponent::GetMaxAcceleration() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterMovementComponent::GetMaxAcceleration);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);

	// Update the Acceleration using the Movement Curve.
	// This allows for fine control over movement behavior at each speed.
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		return Super::GetMaxAcceleration();
	}
	return CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).X;
}

float UALSCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterMovementComponent::GetMaxBrakingDeceleration);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);

	// Update the Deceleration using the Movement Curve.
	// This allows for fine control over movement behavior at each speed.
	if (!IsMovingOnGround() || !CurrentMovementSettings.MovementCurve)
	{
		return Super::GetMaxBrakingDeceleration();
	}
	return CurrentMovementSettings.MovementCurve->GetVectorValue(GetMappedSpeed()).Y;
}

void UALSCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags) // Client only
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterMovementComponent::UpdateFromCompressedFlags);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);

	Super::UpdateFromCompressedFlags(Flags);

	bRequestMovementSettingsChange = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

class FNetworkPredictionData_Client* UALSCharacterMovementComponent::GetPredictionData_Client() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterMovementComponent::GetPredictionData_Client);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);

	check(PawnOwner != nullptr);

	if (!ClientPredictionData)
	{
		UALSCharacterMovementComponent* MutableThis = const_cast<UALSCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_My(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UALSCharacterMovementComponent::FSavedMove_My::Clear()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterMovementComponent::FSavedMove_My::Clear);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);

	Super::Clear();

	bSavedRequestMovementSettingsChange = false;
	SavedAllowedGait = EALSGait::Walking;
}

uint8 UALSCharacterMovementComponent::FSavedMove_My::GetCompressedFlags() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterMovementComponent::FSavedMove_My::GetCompressedFlags);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);

	uint8 Result = Super::GetCompressedFlags();

	if (bSavedRequestMovementSettingsChange)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

void UALSCharacterMovementComponent::FSavedMove_My::SetMoveFor(ACharacter* Character, float InDeltaTime,
                                                               FVector const& NewAccel,
                                                               class FNetworkPredictionData_Client_Character&
                                                               ClientData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterMovementComponent::FSavedMove_My::SetMoveFor);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);

	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UALSCharacterMovementComponent* CharacterMovement = Cast<UALSCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		bSavedRequestMovementSettingsChange = CharacterMovement->bRequestMovementSettingsChange;
		SavedAllowedGait = CharacterMovement->AllowedGait;
	}
}

void UALSCharacterMovementComponent::FSavedMove_My::PrepMoveFor(ACharacter* Character)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterMovementComponent::FSavedMove_My::PrepMoveFor);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);

	Super::PrepMoveFor(Character);

	UALSCharacterMovementComponent* CharacterMovement = Cast<UALSCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		CharacterMovement->AllowedGait = SavedAllowedGait;
	}
}

UALSCharacterMovementComponent::FNetworkPredictionData_Client_My::FNetworkPredictionData_Client_My(
	const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UALSCharacterMovementComponent::FNetworkPredictionData_Client_My::AllocateNewMove()
{
	return MakeShared<FSavedMove_My>();
}

void UALSCharacterMovementComponent::Server_SetAllowedGait_Implementation(const EALSGait NewAllowedGait)
{
	AllowedGait = NewAllowedGait;
}

float UALSCharacterMovementComponent::GetMappedSpeed() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterMovementComponent::GetMappedSpeed);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);

	// Map the character's current speed to the configured movement speeds with a range of 0-3,
	// with 0 = stopped, 1 = the Walk Speed, 2 = the Run Speed, and 3 = the Sprint Speed.
	// This allows us to vary the movement speeds but still use the mapped range in calculations for consistent results

	const float Speed = Velocity.Size2D();
	const float LocWalkSpeed = CurrentMovementSettings.WalkSpeed;
	const float LocRunSpeed = CurrentMovementSettings.RunSpeed;
	const float LocSprintSpeed = CurrentMovementSettings.SprintSpeed;

	if (Speed > LocRunSpeed)
	{
		return FMath::GetMappedRangeValueClamped<float, float>({LocRunSpeed, LocSprintSpeed}, {2.0f, 3.0f}, Speed);
	}

	if (Speed > LocWalkSpeed)
	{
		return FMath::GetMappedRangeValueClamped<float, float>({LocWalkSpeed, LocRunSpeed}, {1.0f, 2.0f}, Speed);
	}

	return FMath::GetMappedRangeValueClamped<float, float>({0.0f, LocWalkSpeed}, {0.0f, 1.0f}, Speed);
}

void UALSCharacterMovementComponent::SetMovementSettings(FALSMovementSettings NewMovementSettings)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterMovementComponent::SetMovementSettings);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);

	// Set the current movement settings from the owner
	CurrentMovementSettings = NewMovementSettings;
	bRequestMovementSettingsChange = true;
}

void UALSCharacterMovementComponent::SetAllowedGait(EALSGait NewAllowedGait)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterMovementComponent::SetAllowedGait);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);

	if (AllowedGait != NewAllowedGait)
	{
		if (PawnOwner->IsLocallyControlled())
		{
			AllowedGait = NewAllowedGait;
			if (GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy)
			{
				Server_SetAllowedGait(NewAllowedGait);
			}
			bRequestMovementSettingsChange = true;
			return;
		}
		if (!PawnOwner->HasAuthority())
		{
			const float UpdateMaxWalkSpeed = CurrentMovementSettings.GetSpeedForGait(AllowedGait);
			MaxWalkSpeed = UpdateMaxWalkSpeed;
			MaxWalkSpeedCrouched = UpdateMaxWalkSpeed;
		}
	}
}

////-																				//
//-								Gravity									        	//
//-																					//


void UALSCharacterMovementComponent::SetNewGravityScale(const float NewGravity)
{
	OldGravityScale = GravityScale;
	GravityScale = NewGravity;
}

void UALSCharacterMovementComponent::ResetGravityScale()
{
	GravityScale = OldGravityScale;
}

void UALSCharacterMovementComponent::SetFixedGravityDirection(const FVector& NewFixedGravityDirection)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (NewFixedGravityDirection.IsZero() ||
	(GravityDirectionMode == EGravityDirectionMode::Fixed &&
	GravityVectorA == NewFixedGravityDirection))
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	bDirtyGravityDirection = true;
	GravityDirectionMode = EGravityDirectionMode::Fixed;
	GravityVectorA = NewFixedGravityDirection;
	GravityDirectionChanged(OldGravityDirectionMode);
}

void UALSCharacterMovementComponent::MulticastSetFixedGravityDirection_Implementation(
	const FVector& NewFixedGravityDirection)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (GravityDirectionMode == EGravityDirectionMode::Fixed &&
	GravityVectorA == NewFixedGravityDirection)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	GravityDirectionMode = EGravityDirectionMode::Fixed;
	GravityVectorA = NewFixedGravityDirection;

	GravityDirectionChanged(OldGravityDirectionMode);

}

void UALSCharacterMovementComponent::SetSplineTangentGravityDirection(AActor* NewGravityActor)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (NewGravityActor == nullptr ||
		(GravityDirectionMode == EGravityDirectionMode::SplineTangent &&
		GravityActor == NewGravityActor))
	{
		return;
	}

	const USplineComponent* Spline = Cast<USplineComponent>(
		NewGravityActor->GetComponentByClass(USplineComponent::StaticClass()));
	if (Spline != nullptr)
	{
		const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

		bDirtyGravityDirection = true;
		GravityDirectionMode = EGravityDirectionMode::SplineTangent;
		GravityActor = NewGravityActor;

		GravityDirectionChanged(OldGravityDirectionMode);
	}

}

void UALSCharacterMovementComponent::MulticastSetSplineTangentGravityDirection_Implementation(AActor* NewGravityActor)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (GravityDirectionMode == EGravityDirectionMode::SplineTangent &&
		GravityActor == NewGravityActor)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	GravityDirectionMode = EGravityDirectionMode::SplineTangent;
	GravityActor = NewGravityActor;

	GravityDirectionChanged(OldGravityDirectionMode);

}

void UALSCharacterMovementComponent::SetPointGravityDirection(const FVector& NewGravityPoint)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (GravityDirectionMode == EGravityDirectionMode::Point &&
		GravityVectorA == NewGravityPoint)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	bDirtyGravityDirection = true;
	GravityDirectionMode = EGravityDirectionMode::Point;
	GravityVectorA = NewGravityPoint;
	GravityActor = nullptr;

	GravityDirectionChanged(OldGravityDirectionMode);

}

void UALSCharacterMovementComponent::MulticastSetPointGravityDirection_Implementation(const FVector& NewGravityPoint)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (GravityDirectionMode == EGravityDirectionMode::Point &&
		GravityVectorA == NewGravityPoint)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	GravityDirectionMode = EGravityDirectionMode::Point;
	GravityVectorA = NewGravityPoint;
	GravityActor = nullptr;

	GravityDirectionChanged(OldGravityDirectionMode);

}

void UALSCharacterMovementComponent::SetPointGravityDirectionFromActor(AActor* NewGravityActor)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (GravityDirectionMode == EGravityDirectionMode::Point &&
		GravityActor == NewGravityActor)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	bDirtyGravityDirection = true;
	GravityDirectionMode = EGravityDirectionMode::Point;
	GravityActor = NewGravityActor;

	GravityDirectionChanged(OldGravityDirectionMode);

}


void UALSCharacterMovementComponent::MulticastSetPointGravityDirectionFromActor_Implementation(AActor* NewGravityActor)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (GravityDirectionMode == EGravityDirectionMode::Point &&
		GravityActor == NewGravityActor)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	GravityDirectionMode = EGravityDirectionMode::Point;
	GravityActor = NewGravityActor;

	GravityDirectionChanged(OldGravityDirectionMode);

}



void UALSCharacterMovementComponent::SetLineGravityDirection(const FVector& NewGravityLineStart,
	const FVector& NewGravityLineEnd)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (NewGravityLineStart == NewGravityLineEnd ||
		(GravityDirectionMode == EGravityDirectionMode::Line &&
		GravityVectorA == NewGravityLineStart && GravityVectorB == NewGravityLineEnd))
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	bDirtyGravityDirection = true;
	GravityDirectionMode = EGravityDirectionMode::Line;
	GravityVectorA = NewGravityLineStart;
	GravityVectorB = NewGravityLineEnd;

	GravityDirectionChanged(OldGravityDirectionMode);

}

void UALSCharacterMovementComponent::MulticastSetLineGravityDirection_Implementation(const FVector& NewGravityLineStart,
																					 const FVector& NewGravityLineEnd)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (GravityDirectionMode == EGravityDirectionMode::Line &&
	GravityVectorA == NewGravityLineStart && GravityVectorB == NewGravityLineEnd)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	GravityDirectionMode = EGravityDirectionMode::Line;
	GravityVectorA = NewGravityLineStart;
	GravityVectorB = NewGravityLineEnd;

	GravityDirectionChanged(OldGravityDirectionMode);

}


void UALSCharacterMovementComponent::SetSegmentGravityDirection(const FVector& NewGravitySegmentStart,
	const FVector& NewGravitySegmentEnd)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (NewGravitySegmentStart == NewGravitySegmentEnd ||
	(GravityDirectionMode == EGravityDirectionMode::Segment &&
	GravityVectorA == NewGravitySegmentStart && GravityVectorB == NewGravitySegmentEnd))
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	bDirtyGravityDirection = true;
	GravityDirectionMode = EGravityDirectionMode::Segment;
	GravityVectorA = NewGravitySegmentStart;
	GravityVectorB = NewGravitySegmentEnd;

	GravityDirectionChanged(OldGravityDirectionMode);


}

void UALSCharacterMovementComponent::MulticastSetSegmentGravityDirection_Implementation(
	const FVector& NewGravitySegmentStart, const FVector& NewGravitySegmentEnd)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (GravityDirectionMode == EGravityDirectionMode::Segment &&
	GravityVectorA == NewGravitySegmentStart && GravityVectorB == NewGravitySegmentEnd)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	GravityDirectionMode = EGravityDirectionMode::Segment;
	GravityVectorA = NewGravitySegmentStart;
	GravityVectorB = NewGravitySegmentEnd;

	GravityDirectionChanged(OldGravityDirectionMode);

}


void UALSCharacterMovementComponent::SetSplineGravityDirection(AActor* NewGravityActor)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (NewGravityActor == nullptr ||
	(GravityDirectionMode == EGravityDirectionMode::Spline &&
	GravityActor == NewGravityActor))
	{
		return;
	}

	const USplineComponent* Spline = Cast<USplineComponent>(
		NewGravityActor->GetComponentByClass(USplineComponent::StaticClass()));
	if (Spline != nullptr)
	{
		const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

		bDirtyGravityDirection = true;
		GravityDirectionMode = EGravityDirectionMode::Spline;
		GravityActor = NewGravityActor;

		GravityDirectionChanged(OldGravityDirectionMode);
	}
}

void UALSCharacterMovementComponent::MulticastSetSplineGravityDirection_Implementation(AActor* NewGravityActor)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (GravityDirectionMode == EGravityDirectionMode::Spline &&
	GravityActor == NewGravityActor)
	{
		return;
	}
	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;
	GravityDirectionMode = EGravityDirectionMode::Spline;
	GravityActor = NewGravityActor;
	GravityDirectionChanged(OldGravityDirectionMode);
}

void UALSCharacterMovementComponent::SetPlaneGravityDirection(const FVector& NewGravityPlaneBase,
	const FVector& NewGravityPlaneNormal)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (NewGravityPlaneNormal.IsZero() ||
	(GravityDirectionMode == EGravityDirectionMode::Plane &&
	GravityVectorA == NewGravityPlaneBase && GravityVectorB == NewGravityPlaneNormal))
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	bDirtyGravityDirection = true;
	GravityDirectionMode = EGravityDirectionMode::Plane;
	GravityVectorA = NewGravityPlaneBase;
	GravityVectorB = NewGravityPlaneNormal;

	GravityDirectionChanged(OldGravityDirectionMode);


}

void UALSCharacterMovementComponent::MulticastSetPlaneGravityDirection_Implementation(
	const FVector& NewGravityPlaneBase, const FVector& NewGravityPlaneNormal)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (GravityDirectionMode == EGravityDirectionMode::Plane &&
	GravityVectorA == NewGravityPlaneBase && GravityVectorB == NewGravityPlaneNormal)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	GravityDirectionMode = EGravityDirectionMode::Plane;
	GravityVectorA = NewGravityPlaneBase;
	GravityVectorB = NewGravityPlaneNormal;

	GravityDirectionChanged(OldGravityDirectionMode);

}


void UALSCharacterMovementComponent::SetSplinePlaneGravityDirection(AActor* NewGravityActor)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (NewGravityActor == nullptr ||
	(GravityDirectionMode == EGravityDirectionMode::SplinePlane &&
	GravityActor == NewGravityActor))
	{
		return;
	}

	const USplineComponent* Spline = Cast<USplineComponent>(
		NewGravityActor->GetComponentByClass(USplineComponent::StaticClass()));
	if (Spline != nullptr)
	{
		const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

		bDirtyGravityDirection = true;
		GravityDirectionMode = EGravityDirectionMode::SplinePlane;
		GravityActor = NewGravityActor;

		GravityDirectionChanged(OldGravityDirectionMode);
	}


}

void UALSCharacterMovementComponent::MulticastSetSplinePlaneGravityDirection_Implementation(AActor* NewGravityActor)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (GravityDirectionMode == EGravityDirectionMode::SplinePlane &&
	GravityActor == NewGravityActor)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	GravityDirectionMode = EGravityDirectionMode::SplinePlane;
	GravityActor = NewGravityActor;

	GravityDirectionChanged(OldGravityDirectionMode);

}



void UALSCharacterMovementComponent::SetBoxGravityDirection(const FVector& NewGravityBoxOrigin,
	const FVector& NewGravityBoxExtent)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (GravityDirectionMode == EGravityDirectionMode::Box &&
	GravityVectorA == NewGravityBoxOrigin && GravityVectorB == NewGravityBoxExtent)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	bDirtyGravityDirection = true;
	GravityDirectionMode = EGravityDirectionMode::Box;
	GravityVectorA = NewGravityBoxOrigin;
	GravityVectorB = NewGravityBoxExtent;
	GravityActor = nullptr;

	GravityDirectionChanged(OldGravityDirectionMode);


}

void UALSCharacterMovementComponent::MulticastSetBoxGravityDirection_Implementation(const FVector& NewGravityBoxOrigin,
																					const FVector& NewGravityBoxExtent)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);

	if (GravityDirectionMode == EGravityDirectionMode::Box &&
	GravityVectorA == NewGravityBoxOrigin && GravityVectorB == NewGravityBoxExtent)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	GravityDirectionMode = EGravityDirectionMode::Box;
	GravityVectorA = NewGravityBoxOrigin;
	GravityVectorB = NewGravityBoxExtent;
	GravityActor = nullptr;

	GravityDirectionChanged(OldGravityDirectionMode);

}



void UALSCharacterMovementComponent::SetBoxGravityDirectionFromActor(AActor* NewGravityActor)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (GravityDirectionMode == EGravityDirectionMode::Box && GravityActor == NewGravityActor)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	bDirtyGravityDirection = true;
	GravityDirectionMode = EGravityDirectionMode::Box;
	GravityActor = NewGravityActor;

	GravityDirectionChanged(OldGravityDirectionMode);


}

void UALSCharacterMovementComponent::MulticastSetBoxGravityDirectionFromActor_Implementation(AActor* NewGravityActor)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);

	if (GravityDirectionMode == EGravityDirectionMode::Box && GravityActor == NewGravityActor)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	GravityDirectionMode = EGravityDirectionMode::Box;
	GravityActor = NewGravityActor;

	GravityDirectionChanged(OldGravityDirectionMode);

}

void UALSCharacterMovementComponent::SetCollisionGravityDirection(AActor* NewGravityActor)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	if (NewGravityActor == nullptr ||
	(GravityDirectionMode == EGravityDirectionMode::Collision &&
	GravityActor == NewGravityActor))
	{
		return;
	}

	const UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(NewGravityActor->GetRootComponent());
	if (Primitive != nullptr)
	{
		const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

		bDirtyGravityDirection = true;
		GravityDirectionMode = EGravityDirectionMode::Collision;
		GravityActor = NewGravityActor;

		GravityDirectionChanged(OldGravityDirectionMode);
	}


}

void UALSCharacterMovementComponent::MulticastSetCollisionGravityDirection_Implementation(AActor* NewGravityActor)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);

	if (GravityDirectionMode == EGravityDirectionMode::Collision && GravityActor == NewGravityActor)
	{
		return;
	}

	const EGravityDirectionMode OldGravityDirectionMode = GravityDirectionMode;

	GravityDirectionMode = EGravityDirectionMode::Collision;
	GravityActor = NewGravityActor;

	GravityDirectionChanged(OldGravityDirectionMode);

}

void UALSCharacterMovementComponent::GravityDirectionChanged(const EGravityDirectionMode OldGravityDirectionMode)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	OnGravityDirectionChanged.Broadcast(OldGravityDirectionMode, GravityDirectionMode);

	// Call owner delegate
	// IALSCharacterInterface* Character = Cast<IALSCharacterInterface>(CharacterOwner);
	// if (Character != nullptr)
	// {
	// 	Character->GravityDirectionChanged(OldGravityDirectionMode, GravityDirectionMode);
	// }

}


bool UALSCharacterMovementComponent::GetIsWalking() const
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	return IsWalking();
}

void UALSCharacterMovementComponent::LaunchCharacter(const FVector& LaunchVel)
{
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Movement_Gravity);
	Launch(LaunchVel);
}

FVector UALSCharacterMovementComponent::GetGravityDirection() const
{
    switch (GravityDirectionMode)
    {
        case EGravityDirectionMode::Fixed:
            // Gravity is in a fixed direction (GravityVectorA).
            return GravityVectorA.GetSafeNormal();

        case EGravityDirectionMode::Point:
            // Gravity pulls towards a specific point (like a planet center).
            if (GravityActor)
            {
                return (GravityActor->GetActorLocation() - GetOwner()->GetActorLocation()).GetSafeNormal();
            }
            return (GravityVectorA - GetOwner()->GetActorLocation()).GetSafeNormal();

        case EGravityDirectionMode::Line:
        case EGravityDirectionMode::Segment:
            // Gravity is along a line or segment between two points (GravityVectorA and GravityVectorB).
            return (GravityVectorB - GravityVectorA).GetSafeNormal();

		case EGravityDirectionMode::Spline:
        case EGravityDirectionMode::SplineTangent:
        case EGravityDirectionMode::SplinePlane:
            // Gravity follows the tangent of a spline (typically along a path).
            if (const USplineComponent* Spline = Cast<USplineComponent>(GravityActor->GetComponentByClass(USplineComponent::StaticClass())))
            {
                FVector ClosestPoint = Spline->FindLocationClosestToWorldLocation(GetOwner()->GetActorLocation(), ESplineCoordinateSpace::World);
                return Spline->FindTangentClosestToWorldLocation(ClosestPoint, ESplineCoordinateSpace::World).GetSafeNormal();
            }
            break;

        case EGravityDirectionMode::Plane:
            // Gravity is along a plane normal (GravityVectorB is the normal).
            return GravityVectorB.GetSafeNormal();

        case EGravityDirectionMode::Box:
            // Gravity is towards the center of a box.
            return (GravityVectorA - GetOwner()->GetActorLocation()).GetSafeNormal();

        case EGravityDirectionMode::Collision:
            // Gravity is derived from a collision (e.g., based on the normal of the collision surface).
            if (GravityActor)
            {
                const UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(GravityActor->GetRootComponent());
                if (PrimitiveComponent)
                {
                    return PrimitiveComponent->GetComponentLocation().GetSafeNormal();
                }
            }
            break;

        default:
            break;
    }
    // Fallback to world gravity (e.g., downwards) if no specific direction is set.
    return FVector(0.0f, 0.0f, -1.0f);
}

void UALSCharacterMovementComponent::UpdateGravity()
{
	if (!bAlignGravityToBase || !IsMovingOnGround())
	{
		return;
	}

	switch (GravityDirectionMode)
	{
	case EGravityDirectionMode::Fixed:
		{
			if (!CurrentFloor.HitResult.ImpactNormal.IsZero())
			{
				// Set the fixed gravity direction to reversed floor normal vector
				SetFixedGravityDirection(CurrentFloor.HitResult.ImpactNormal * -1.0f);
			}

			break;
		}

	case EGravityDirectionMode::Point:
		{
			if (CurrentFloor.HitResult.GetActor() != nullptr)
			{
				// Set the point gravity direction from base
				SetPointGravityDirectionFromActor(CurrentFloor.HitResult.GetActor());
			}

			break;
		}

	case EGravityDirectionMode::Box:
		{
			if (CurrentFloor.HitResult.GetActor() != nullptr)
			{
				// Set the box gravity direction from base
				SetBoxGravityDirectionFromActor(CurrentFloor.HitResult.GetActor());
			}

			break;
		}
	case EGravityDirectionMode::Collision:
	default:
		{
			if (CurrentFloor.HitResult.GetActor() != nullptr)
			{
				// Set the collision gravity direction from base
				SetCollisionGravityDirection(CurrentFloor.HitResult.GetActor());
			}

			break;
		}
	}

}

void UALSCharacterMovementComponent::ReplicateGravityToClients()
{
	if (bDirtyGravityDirection)
	{
		// Replicate gravity direction to clients
		switch (GravityDirectionMode)
		{
		case EGravityDirectionMode::Fixed:
			{
				MulticastSetFixedGravityDirection(GravityVectorA);
				break;
			}

		case EGravityDirectionMode::SplineTangent:
			{
				MulticastSetSplineTangentGravityDirection(GravityActor);
				break;
			}

		case EGravityDirectionMode::Point:
			{
				MulticastSetPointGravityDirection(GravityVectorA);
				break;
			}

		case EGravityDirectionMode::Line:
			{
				MulticastSetLineGravityDirection(GravityVectorA, GravityVectorB);
				break;
			}

		case EGravityDirectionMode::Segment:
			{
				MulticastSetSegmentGravityDirection(GravityVectorA, GravityVectorB);
				break;
			}

		case EGravityDirectionMode::Spline:
			{
				MulticastSetSplineGravityDirection(GravityActor);
				break;
			}

		case EGravityDirectionMode::Plane:
			{
				MulticastSetPlaneGravityDirection(GravityVectorA, GravityVectorB);
				break;
			}

		case EGravityDirectionMode::SplinePlane:
			{
				MulticastSetSplinePlaneGravityDirection(GravityActor);
				break;
			}

		case EGravityDirectionMode::Box:
			{
				MulticastSetBoxGravityDirection(GravityVectorA, GravityVectorB);
				break;
			}

		case EGravityDirectionMode::Collision:
			{
				MulticastSetCollisionGravityDirection(GravityActor);
				break;
			}
		}

		bDirtyGravityDirection = false;
	}
	if (OldGravityScale != GravityScale)
	{
		// Replicate gravity scale to clients
		MulticastSetGravityScale(GravityScale);
		OldGravityScale = GravityScale;
	}
}

void UALSCharacterMovementComponent::SetAlignGravityToBase(const bool bNewAlignGravityToBase)
{
	if (bAlignGravityToBase == bNewAlignGravityToBase)
	{
		return;
	}

	bAlignGravityToBase = bNewAlignGravityToBase;

	if (GetShouldReplicateGravity())
	{
		if (!bAlignGravityToBase)
		{
			MulticastDisableAlignGravityToBase();
		}
		else
		{
			MulticastEnableAlignGravityToBase();
		}
	}

}

void UALSCharacterMovementComponent::MulticastSetGravityScale_Implementation(const float NewGravityScale)
{
	GravityScale = NewGravityScale;
}

void UALSCharacterMovementComponent::MulticastDisableAlignGravityToBase_Implementation()
{
	bAlignGravityToBase = false;
}

void UALSCharacterMovementComponent::MulticastEnableAlignGravityToBase_Implementation()
{
	bAlignGravityToBase = true;
}

bool UALSCharacterMovementComponent::GetShouldReplicateGravity() const
{
	return (!bDisableGravityReplication && CharacterOwner != nullptr &&
	CharacterOwner->HasAuthority() && GetNetMode() != ENetMode::NM_Standalone);

}








