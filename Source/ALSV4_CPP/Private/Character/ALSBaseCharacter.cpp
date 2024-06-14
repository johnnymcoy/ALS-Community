// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community


#include "Character/ALSBaseCharacter.h"
#include "Character/Animation/ALSCharacterAnimInstance.h"
#include "Character/Animation/ALSPlayerCameraBehavior.h"
#include "Library/ALSMathLibrary.h"
#include "Components/ALSDebugComponent.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveFloat.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "NavAreas/NavArea_Obstacle.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"


const FName NAME_FP_Camera(TEXT("FP_Camera"));
const FName NAME_ADS_Camera(TEXT("ADS_Camera"));
const FName NAME_Pelvis(TEXT("Pelvis"));
const FName NAME_RagdollPose(TEXT("RagdollPose"));
const FName NAME_RotationAmount(TEXT("RotationAmount"));
const FName NAME_YawOffset(TEXT("YawOffset"));
const FName NAME_pelvis(TEXT("pelvis"));
const FName NAME_root(TEXT("root"));
const FName NAME_spine_03(TEXT("spine_03"));

DECLARE_CYCLE_STAT(TEXT("ALS Base Character (All Functions)"), STATGROUP_ALS_Base_Character, STATGROUP_ALS);
DECLARE_CYCLE_STAT(TEXT("ALS Base Character Tick"), STATGROUP_ALS_Base_Character_Tick, STATGROUP_ALS);


AALSBaseCharacter::AALSBaseCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UALSCharacterMovementComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	bReplicates = true;
	SetReplicatingMovement(true);
	
	SetupCapsuleComponent();	
	SetupMeshComponent();
}

void AALSBaseCharacter::PostInitializeComponents()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::PostInitializeComponents);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	Super::PostInitializeComponents();
	MyCharacterMovementComponent = Cast<UALSCharacterMovementComponent>(Super::GetMovementComponent());
}

void AALSBaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AALSBaseCharacter, TargetRagdollLocation);
	DOREPLIFETIME_CONDITION(AALSBaseCharacter, ReplicatedCurrentAcceleration, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AALSBaseCharacter, ReplicatedControlRotation, COND_SkipOwner);

	DOREPLIFETIME(AALSBaseCharacter, DesiredGait);
	DOREPLIFETIME_CONDITION(AALSBaseCharacter, DesiredStance, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AALSBaseCharacter, DesiredRotationMode, COND_SkipOwner);

	DOREPLIFETIME_CONDITION(AALSBaseCharacter, RotationMode, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AALSBaseCharacter, OverlayState, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AALSBaseCharacter, ViewMode, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(AALSBaseCharacter, VisibleMesh, COND_SkipOwner);
}

void AALSBaseCharacter::OnBreakfall_Implementation()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::OnBreakfall_Implementation);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	Replicated_PlayMontage(GetRollAnimation(), 1.35);
}

void AALSBaseCharacter::Replicated_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::Replicated_PlayMontage_Implementation);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);
	// Roll: Simply play a Root Motion Montage.
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage, PlayRate);
	}

	Server_PlayMontage(Montage, PlayRate);
}

void AALSBaseCharacter::BeginPlay()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::BeginPlay);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	Super::BeginPlay();

	// If we're in networked game, disable curved movement
	bEnableNetworkOptimizations = !IsNetMode(NM_Standalone);
	if(bForceEnableNetworkOptimizationsOff){bEnableNetworkOptimizations = false;}

	// Make sure the mesh and animbp update after the CharacterBP to ensure it gets the most recent values.
	GetMesh()->AddTickPrerequisiteActor(this);

	// Set the Movement Model
	SetMovementModel();

	// Force update states to use the initial desired values.
	ForceUpdateCharacterState();

	if (Stance == EALSStance::Standing)
	{
		UnCrouch();
	}
	else if (Stance == EALSStance::Crouching)
	{
		Crouch();
	}

	// Set default rotation values.
	TargetRotation = GetActorRotation();
	LastVelocityRotation = TargetRotation;
	LastMovementInputRotation = TargetRotation;

	if (GetMesh()->GetAnimInstance() && GetLocalRole() == ROLE_SimulatedProxy)
	{
		GetMesh()->GetAnimInstance()->SetRootMotionMode(ERootMotionMode::IgnoreRootMotion);
	}

	MyCharacterMovementComponent->SetMovementSettings(GetTargetMovementSettings());

	ALSDebugComponent = FindComponentByClass<UALSDebugComponent>();
}

void AALSBaseCharacter::Ragdoll()
{
	RagdollStart();
}

UPrimitiveComponent* AALSBaseCharacter::GetPrimitiveComponent()
{
	return Cast<UPrimitiveComponent>(RootComponent);
}

void AALSBaseCharacter::Tick(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::Tick);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character_Tick);

	Super::Tick(DeltaTime);

	// Set required values
	SetEssentialValues(DeltaTime);

	if (MovementState == EALSMovementState::Grounded)
	{
		UpdateCharacterMovement();
		UpdateGroundedRotation(DeltaTime);
	}
	else if (MovementState == EALSMovementState::InAir)
	{
		UpdateInAirRotation(DeltaTime);
	}
	else if (MovementState == EALSMovementState::Ragdoll)
	{
		RagdollUpdate(DeltaTime);
	}

	// Cache values
	PreviousVelocity = GetVelocity();
	PreviousAimYaw = AimingRotation.Yaw;
}

void AALSBaseCharacter::RagdollStart()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::RagdollStart);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);
	if(GetWorld() == nullptr){return;} //- Added world Check..
	if (RagdollStateChangedDelegate.IsBound())
	{
		RagdollStateChangedDelegate.Broadcast(true);
	}

	/** When Networked, disables replicate movement reset TargetRagdollLocation and ServerRagdollPull variable
	and if the host is a dedicated server, change character mesh optimisation option to avoid z-location bug*/
	MyCharacterMovementComponent->bIgnoreClientMovementErrorChecksAndCorrection = 1;
	if (UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	{
		DefVisBasedTickOp = GetMesh()->VisibilityBasedAnimTickOption;
		GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
	}
	TargetRagdollLocation = GetMesh()->GetSocketLocation(NAME_Pelvis);
	ServerRagdollPull = 0;

	// Disable URO
	bPreRagdollURO = GetMesh()->bEnableUpdateRateOptimizations;
	GetMesh()->bEnableUpdateRateOptimizations = false;

	// Step 1: Clear the Character Movement Mode and set the Movement State to Ragdoll
	GetCharacterMovement()->SetMovementMode(MOVE_None);
	SetMovementState(EALSMovementState::Ragdoll);

	// Step 2: Disable capsule collision and enable mesh physics simulation starting from the pelvis.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionObjectType(ECC_PhysicsBody);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetAllBodiesBelowSimulatePhysics(NAME_Pelvis, true, true);

	// Step 3: Stop any active montages.
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Stop(0.2f);
	}

	// Fixes character mesh is showing default A pose for a split-second just before ragdoll ends in listen server games
	GetMesh()->bOnlyAllowAutonomousTickPose = true;

	SetReplicateMovement(false);
}

void AALSBaseCharacter::RagdollEnd()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::RagdollEnd);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);
	/** Re-enable Replicate Movement and if the host is a dedicated server set mesh visibility based anim
	tick option back to default*/

	if (UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	{
		GetMesh()->VisibilityBasedAnimTickOption = DefVisBasedTickOp;
	}

	GetMesh()->bEnableUpdateRateOptimizations = bPreRagdollURO;

	// Revert back to default settings
	MyCharacterMovementComponent->bIgnoreClientMovementErrorChecksAndCorrection = 0;
	GetMesh()->bOnlyAllowAutonomousTickPose = false;
	SetReplicateMovement(true);

	// Step 1: Save a snapshot of the current Ragdoll Pose for use in AnimGraph to blend out of the ragdoll
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->SavePoseSnapshot(NAME_RagdollPose);
	}

	// Step 2: If the ragdoll is on the ground, set the movement mode to walking and play a Get Up animation.
	// If not, set the movement mode to falling and update the character movement velocity to match the last ragdoll velocity.
	if (bRagdollOnGround)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		if (GetMesh()->GetAnimInstance())
		{
			GetMesh()->GetAnimInstance()->Montage_Play(GetGetUpAnimation(bRagdollFaceUp), 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		}
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		GetCharacterMovement()->Velocity = LastRagdollVelocity;
	}

	// Step 3: Re-Enable capsule collision, and disable physics simulation on the mesh.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GetMesh()->SetCollisionObjectType(ECC_Pawn);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	GetMesh()->SetAllBodiesSimulatePhysics(false);

	if (RagdollStateChangedDelegate.IsBound())
	{
		RagdollStateChangedDelegate.Broadcast(false);
	}
}

void AALSBaseCharacter::Server_SetMeshLocationDuringRagdoll_Implementation(FVector MeshLocation)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::Server_SetMeshLocationDuringRagdoll_Implementation);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);
	TargetRagdollLocation = MeshLocation;
}

void AALSBaseCharacter::SetMovementState(const EALSMovementState NewState, bool bForce)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetMovementState);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);
	if (bForce || MovementState != NewState)
	{
		PrevMovementState = MovementState;
		MovementState = NewState;
		OnMovementStateChanged(PrevMovementState);
	}
}

void AALSBaseCharacter::SetMovementAction(const EALSMovementAction NewAction, bool bForce)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetMovementAction);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);
	if (bForce || MovementAction != NewAction)
	{
		const EALSMovementAction Prev = MovementAction;
		MovementAction = NewAction;
		OnMovementActionChanged(Prev);
	}
}

void AALSBaseCharacter::SetStance(const EALSStance NewStance, bool bForce)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetStance);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);
	if (bForce || Stance != NewStance)
	{
		const EALSStance Prev = Stance;
		Stance = NewStance;
		OnStanceChanged(Prev);
	}
}

void AALSBaseCharacter::SetOverlayOverrideState(int32 NewState)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetOverlayOverrideState);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);
	OverlayOverrideState = NewState;
}

void AALSBaseCharacter::SetGait(const EALSGait NewGait, bool bForce)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetGait);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);
	if (bForce || Gait != NewGait)
	{
		const EALSGait Prev = Gait;
		Gait = NewGait;
		OnGaitChanged(Prev);
	}
}


void AALSBaseCharacter::SetDesiredStance(EALSStance NewStance)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetDesiredStance);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	DesiredStance = NewStance;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredStance(NewStance);
	}
}

void AALSBaseCharacter::Server_SetDesiredStance_Implementation(EALSStance NewStance)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::Server_SetDesiredStance_Implementation);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);
	SetDesiredStance(NewStance);
}

void AALSBaseCharacter::SetDesiredGait(const EALSGait NewGait)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetDesiredGait);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	DesiredGait = NewGait;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredGait(NewGait);
	}
}

void AALSBaseCharacter::Server_SetDesiredGait_Implementation(EALSGait NewGait)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::Server_SetDesiredGait_Implementation);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	SetDesiredGait(NewGait);
}

void AALSBaseCharacter::SetDesiredRotationMode(EALSRotationMode NewRotMode)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetDesiredRotationMode);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	DesiredRotationMode = NewRotMode;
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetDesiredRotationMode(NewRotMode);
	}
}

void AALSBaseCharacter::Server_SetDesiredRotationMode_Implementation(EALSRotationMode NewRotMode)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::Server_SetDesiredRotationMode_Implementation);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	SetDesiredRotationMode(NewRotMode);
}

void AALSBaseCharacter::SetRotationMode(const EALSRotationMode NewRotationMode, bool bForce)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetRotationMode);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	if (bForce || RotationMode != NewRotationMode)
	{
		const EALSRotationMode Prev = RotationMode;
		RotationMode = NewRotationMode;
		OnRotationModeChanged(Prev);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetRotationMode(NewRotationMode, bForce);
		}
	}
}


void AALSBaseCharacter::Server_SetRotationMode_Implementation(EALSRotationMode NewRotationMode, bool bForce)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::Server_SetRotationMode_Implementation);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	SetRotationMode(NewRotationMode, bForce);
}

void AALSBaseCharacter::SetViewMode(const EALSViewMode NewViewMode, bool bForce)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetViewMode);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	if (bForce || ViewMode != NewViewMode)
	{
		const EALSViewMode Prev = ViewMode;
		ViewMode = NewViewMode;
		OnViewModeChanged(Prev);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetViewMode(NewViewMode, bForce);
		}
	}
}

void AALSBaseCharacter::Server_SetViewMode_Implementation(EALSViewMode NewViewMode, bool bForce)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::Server_SetViewMode_Implementation);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	SetViewMode(NewViewMode, bForce);
}

void AALSBaseCharacter::SetOverlayState(const EALSOverlayState NewState, bool bForce)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetOverlayState);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	if (bForce || OverlayState != NewState)
	{
		const EALSOverlayState Prev = OverlayState;
		OverlayState = NewState;
		OnOverlayStateChanged(Prev);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetOverlayState(NewState, bForce);
		}
	}
}

void AALSBaseCharacter::SetGroundedEntryState(EALSGroundedEntryState NewState)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetGroundedEntryState);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	GroundedEntryState = NewState;
}


void AALSBaseCharacter::Server_SetOverlayState_Implementation(EALSOverlayState NewState, bool bForce)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::Server_SetOverlayState_Implementation);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	SetOverlayState(NewState, bForce);
}

void AALSBaseCharacter::EventOnLanded()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::EventOnLanded);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	const float VelZ = FMath::Abs(GetCharacterMovement()->Velocity.Z);

	if (bRagdollOnLand && VelZ > RagdollOnLandVelocity)
	{
		ReplicatedRagdollStart();
	}
	else if (bBreakfallOnLand && bHasMovementInput && VelZ >= BreakfallOnLandVelocity)
	{
		OnBreakfall();
	}
	else
	{
		GetCharacterMovement()->BrakingFrictionFactor = bHasMovementInput ? 0.5f : 3.0f;

		// After 0.5 secs, reset braking friction factor to zero
		GetWorldTimerManager().SetTimer(OnLandedFrictionResetTimer, this,
		                                &AALSBaseCharacter::OnLandFrictionReset, 0.5f, false);
	}
}

void AALSBaseCharacter::Multicast_OnLanded_Implementation()
{
	if (!IsLocallyControlled())
	{
		EventOnLanded();
	}
}

void AALSBaseCharacter::EventOnJumped()
{
	// Set the new In Air Rotation to the velocity rotation if speed is greater than 100.
	InAirRotation = Speed > 100.0f ? LastVelocityRotation : GetActorRotation();

	OnJumpedDelegate.Broadcast();
}

void AALSBaseCharacter::Server_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage, PlayRate);
	}

	ForceNetUpdate();
	Multicast_PlayMontage(Montage, PlayRate);
}

void AALSBaseCharacter::Multicast_PlayMontage_Implementation(UAnimMontage* Montage, float PlayRate)
{
	if (GetMesh()->GetAnimInstance() && !IsLocallyControlled())
	{
		GetMesh()->GetAnimInstance()->Montage_Play(Montage, PlayRate);
	}
}

void AALSBaseCharacter::Multicast_OnJumped_Implementation()
{
	if (!IsLocallyControlled())
	{
		EventOnJumped();
	}
}

void AALSBaseCharacter::Server_RagdollStart_Implementation()
{
	Multicast_RagdollStart();
}

void AALSBaseCharacter::Multicast_RagdollStart_Implementation()
{
	RagdollStart();
}

void AALSBaseCharacter::Server_RagdollEnd_Implementation(FVector CharacterLocation)
{
	Multicast_RagdollEnd(CharacterLocation);
}

void AALSBaseCharacter::Multicast_RagdollEnd_Implementation(FVector CharacterLocation)
{
	RagdollEnd();
}

void AALSBaseCharacter::SetActorLocationAndTargetRotation(FVector NewLocation, FRotator NewRotation)
{
	SetActorLocationAndRotation(NewLocation, NewRotation);
	TargetRotation = NewRotation;
}

void AALSBaseCharacter::SetMovementModel()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetMovementModel);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	if (ensureMsgf(!MovementModel.IsNull(), TEXT("Did you forget to fill in MovementModel information?")))
	{
		const FString ContextString = GetFullName();
		FALSMovementStateSettings* OutRow = MovementModel.DataTable->FindRow<FALSMovementStateSettings>(MovementModel.RowName, ContextString);
		if (ensureMsgf(OutRow, TEXT("Failed to find row")))
		{
			MovementData = *OutRow;
			return;
		}
	}
	MovementData = {};
}

void AALSBaseCharacter::ForceUpdateCharacterState()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::ForceUpdateCharacterState);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	SetGait(DesiredGait, true);
	SetStance(DesiredStance, true);
	SetRotationMode(DesiredRotationMode, true);
	SetViewMode(ViewMode, true);
	SetOverlayState(OverlayState, true);
	SetMovementState(MovementState, true);
	SetMovementAction(MovementAction, true);
}

FALSMovementSettings AALSBaseCharacter::GetTargetMovementSettings() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::GetTargetMovementSettings);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	if (RotationMode == EALSRotationMode::VelocityDirection)
	{
		if (Stance == EALSStance::Standing)
		{
			return MovementData.VelocityDirection.Standing;
		}
		if (Stance == EALSStance::Crouching)
		{
			return MovementData.VelocityDirection.Crouching;
		}
	}
	else if (RotationMode == EALSRotationMode::LookingDirection)
	{
		if (Stance == EALSStance::Standing)
		{
			return MovementData.LookingDirection.Standing;
		}
		if (Stance == EALSStance::Crouching)
		{
			return MovementData.LookingDirection.Crouching;
		}
	}
	else if (RotationMode == EALSRotationMode::Aiming)
	{
		if (Stance == EALSStance::Standing)
		{
			return MovementData.Aiming.Standing;
		}
		if (Stance == EALSStance::Crouching)
		{
			return MovementData.Aiming.Crouching;
		}
	}

	// Default to velocity dir standing
	return MovementData.VelocityDirection.Standing;
}

bool AALSBaseCharacter::CanSprint() const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::CanSprint);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	// Determine if the character is currently able to sprint based on the Rotation mode and current acceleration
	// (input) rotation. If the character is in the Looking Rotation mode, only allow sprinting if there is full
	// movement input and it is faced forward relative to the camera + or - 50 degrees.

	if (!bHasMovementInput || RotationMode == EALSRotationMode::Aiming)
	{
		return false;
	}

	const bool bValidInputAmount = MovementInputAmount > 0.9f;

	if (RotationMode == EALSRotationMode::VelocityDirection)
	{
		return bValidInputAmount;
	}

	if (RotationMode == EALSRotationMode::LookingDirection)
	{
		const FRotator AccRot = ReplicatedCurrentAcceleration.ToOrientationRotator();
		FRotator Delta = AccRot - AimingRotation;
		Delta.Normalize();

		return bValidInputAmount && FMath::Abs(Delta.Yaw) < 50.0f;
	}

	return false;
}

FVector AALSBaseCharacter::GetMovementInput() const
{
	return ReplicatedCurrentAcceleration;
}

float AALSBaseCharacter::GetAnimCurveValue(FName CurveName) const
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::GetAnimCurveValue);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	if(GetMesh() != nullptr && GetMesh()->GetAnimInstance())
	{
		return GetMesh()->GetAnimInstance()->GetCurveValue(CurveName);
	}

	return 0.0f;
}

void AALSBaseCharacter::SetVisibleMesh(USkeletalMesh* NewVisibleMesh)
{
	if (VisibleMesh != NewVisibleMesh)
	{
		const USkeletalMesh* Prev = VisibleMesh;
		VisibleMesh = NewVisibleMesh;
		OnVisibleMeshChanged(Prev);

		if (GetLocalRole() != ROLE_Authority)
		{
			Server_SetVisibleMesh(NewVisibleMesh);
		}
	}
}

void AALSBaseCharacter::Server_SetVisibleMesh_Implementation(USkeletalMesh* NewVisibleMesh)
{
	SetVisibleMesh(NewVisibleMesh);
}

void AALSBaseCharacter::SetRightShoulder(const bool bNewRightShoulder)
{
	bRightShoulder = bNewRightShoulder;
	if (CameraBehavior)
	{
		CameraBehavior->bRightShoulder = bRightShoulder;
	}
}

bool AALSBaseCharacter::GetCanAimDownSights() const
{
	if(RotationMode != EALSRotationMode::Aiming)
	{
		return false;
	}
	return true;
}

void AALSBaseCharacter::SetAimDownSights(const bool bNewAimDownSights)
{
	if(GetCanAimDownSights() && bNewAimDownSights)
	{
		bAimDownSights = bNewAimDownSights;
		if(CameraBehavior != nullptr)
		{
			CameraBehavior->bAimDownSights = bAimDownSights;
		}
	}
	else
	{
		bAimDownSights = false;
		if(CameraBehavior != nullptr)
		{
			CameraBehavior->bAimDownSights = false;
		}

	}
}

ECollisionChannel AALSBaseCharacter::GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius)
{
	TraceOrigin = GetActorLocation();
	TraceRadius = 10.0f;
	return ECC_Visibility;
}

FTransform AALSBaseCharacter::GetThirdPersonPivotTarget()
{
	return GetActorTransform();
}

FVector AALSBaseCharacter::GetFirstPersonCameraTarget()
{
	if(GetMesh() == nullptr){return FVector::ZeroVector;}
	return GetMesh()->GetSocketLocation(NAME_FP_Camera);
}

//@TODO maybe remove? 
FVector AALSBaseCharacter::GetAimDownSightCameraTarget() const
{
	if(GetMesh() == nullptr){return FVector::ZeroVector;}
	return GetMesh()->GetSocketLocation(NAME_ADS_Camera);
}

float AALSBaseCharacter::GetAimDownSightFOV() const
{
	return FirstPersonFOV;
}

void AALSBaseCharacter::GetCameraParameters(float& TPFOVOut, float& FPFOVOut, bool& bRightShoulderOut) const
{
	TPFOVOut = ThirdPersonFOV;
	FPFOVOut = FirstPersonFOV;
	bRightShoulderOut = bRightShoulder;
}

void AALSBaseCharacter::RagdollUpdate(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::RagdollUpdate);
    SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	GetMesh()->bOnlyAllowAutonomousTickPose = false;

	// Set the Last Ragdoll Velocity.
	const FVector NewRagdollVel = GetMesh()->GetPhysicsLinearVelocity(NAME_root);
	LastRagdollVelocity = (NewRagdollVel != FVector::ZeroVector || IsLocallyControlled())
		                      ? NewRagdollVel
		                      : LastRagdollVelocity / 2;

	// Use the Ragdoll Velocity to scale the ragdoll's joint strength for physical animation.
	const float SpringValue = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 1000.0f}, {0.0f, 25000.0f},
	                                                            LastRagdollVelocity.Size());
	GetMesh()->SetAllMotorsAngularDriveParams(SpringValue, 0.0f, 0.0f, false);

	// Disable Gravity if falling faster than -4000 to prevent continual acceleration.
	// This also prevents the ragdoll from going through the floor.
	const bool bEnableGrav = LastRagdollVelocity.Z > -4000.0f;
	GetMesh()->SetEnableGravity(bEnableGrav);

	// Update the Actor location to follow the ragdoll.
	SetActorLocationDuringRagdoll(DeltaTime);
}

void AALSBaseCharacter::SetActorLocationDuringRagdoll(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetActorLocationDuringRagdoll);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	if (IsLocallyControlled())
	{
		// Set the pelvis as the target location.
		TargetRagdollLocation = GetMesh()->GetSocketLocation(NAME_Pelvis);
		if (!HasAuthority())
		{
			Server_SetMeshLocationDuringRagdoll(TargetRagdollLocation);
		}
	}

	// Determine whether the ragdoll is facing up or down and set the target rotation accordingly.
	const FRotator PelvisRot = GetMesh()->GetSocketRotation(NAME_Pelvis);

	if (bReversedPelvis) {
		bRagdollFaceUp = PelvisRot.Roll > 0.0f;
	} else
	{
		bRagdollFaceUp = PelvisRot.Roll < 0.0f;
	}


	const FRotator TargetRagdollRotation(0.0f, bRagdollFaceUp ? PelvisRot.Yaw - 180.0f : PelvisRot.Yaw, 0.0f);

	// Trace downward from the target location to offset the target location,
	// preventing the lower half of the capsule from going through the floor when the ragdoll is laying on the ground.
	const FVector TraceVect(TargetRagdollLocation.X, TargetRagdollLocation.Y,
	                        TargetRagdollLocation.Z - GetCapsuleComponent()->GetScaledCapsuleHalfHeight());

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	FHitResult HitResult;
	const bool bHit = World->LineTraceSingleByChannel(HitResult, TargetRagdollLocation, TraceVect,
	                                                  ECC_Visibility, Params);

	if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
	{
		UALSDebugComponent::DrawDebugLineTraceSingle(World,
		                                             TargetRagdollLocation,
		                                             TraceVect,
		                                             EDrawDebugTrace::Type::ForOneFrame,
		                                             bHit,
		                                             HitResult,
		                                             FLinearColor::Red,
		                                             FLinearColor::Green,
		                                             1.0f);
	}

	bRagdollOnGround = HitResult.IsValidBlockingHit();
	FVector NewRagdollLoc = TargetRagdollLocation;

	if (bRagdollOnGround)
	{
		const float ImpactDistZ = FMath::Abs(HitResult.ImpactPoint.Z - HitResult.TraceStart.Z);
		NewRagdollLoc.Z += GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - ImpactDistZ + 2.0f;
	}
	if (!IsLocallyControlled())
	{
		ServerRagdollPull = FMath::FInterpTo(ServerRagdollPull, 750.0f, DeltaTime, 0.6f);
		float RagdollSpeed = FVector(LastRagdollVelocity.X, LastRagdollVelocity.Y, 0).Size();
		/** Check if Socket exists, if not use other case	*/
		FName RagdollPelvisSocket = GetMesh()->DoesSocketExist(NAME_pelvis) ? NAME_pelvis : NAME_Pelvis;
		//FName RagdollSocketPullName = RagdollSpeed > 300 ? NAME_spine_03 : NAME_pelvis;
		FName RagdollSocketPullName = RagdollSpeed > 300 ? NAME_spine_03 : RagdollPelvisSocket;
		GetMesh()->AddForce(
			(TargetRagdollLocation - GetMesh()->GetSocketLocation(RagdollSocketPullName)) * ServerRagdollPull,
			RagdollSocketPullName, true);
	}
	SetActorLocationAndTargetRotation(bRagdollOnGround ? NewRagdollLoc : TargetRagdollLocation, TargetRagdollRotation);
}

void AALSBaseCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::OnMovementModeChanged);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	// Use the Character Movement Mode changes to set the Movement States to the right values. This allows you to have
	// a custom set of movement states but still use the functionality of the default character movement component.

	if (GetCharacterMovement()->MovementMode == MOVE_Walking ||
		GetCharacterMovement()->MovementMode == MOVE_NavWalking)
	{
		SetMovementState(EALSMovementState::Grounded);
	}
	else if (GetCharacterMovement()->MovementMode == MOVE_Falling)
	{
		SetMovementState(EALSMovementState::InAir);
	}
}

void AALSBaseCharacter::OnMovementStateChanged(const EALSMovementState PreviousState)
{
	if (MovementState == EALSMovementState::InAir)
	{
		if (MovementAction == EALSMovementAction::None)
		{
			// If the character enters the air, set the In Air Rotation and uncrouch if crouched.
			InAirRotation = GetActorRotation();
			if (Stance == EALSStance::Crouching)
			{
				UnCrouch();
			}
		}
		else if (MovementAction == EALSMovementAction::Rolling)
		{
			// If the character is currently rolling, enable the ragdoll.
			ReplicatedRagdollStart();
		}
	}

	if (CameraBehavior)
	{
		CameraBehavior->MovementState = MovementState;
	}
}

void AALSBaseCharacter::OnMovementActionChanged(const EALSMovementAction PreviousAction)
{
	// Make the character crouch if performing a roll.
	if (MovementAction == EALSMovementAction::Rolling)
	{
		Crouch();
	}

	if (PreviousAction == EALSMovementAction::Rolling)
	{
		if (DesiredStance == EALSStance::Standing)
		{
			UnCrouch();
		}
		else if (DesiredStance == EALSStance::Crouching)
		{
			Crouch();
		}
	}

	if (CameraBehavior)
	{
		CameraBehavior->MovementAction = MovementAction;
	}
}

void AALSBaseCharacter::OnStanceChanged(const EALSStance PreviousStance)
{
	if (CameraBehavior)
	{
		CameraBehavior->Stance = Stance;
	}

	MyCharacterMovementComponent->SetMovementSettings(GetTargetMovementSettings());
}

void AALSBaseCharacter::OnRotationModeChanged(EALSRotationMode PreviousRotationMode)
{
	if (RotationMode == EALSRotationMode::VelocityDirection && ViewMode == EALSViewMode::FirstPerson)
	{
		// If the new rotation mode is Velocity Direction and the character is in First Person,
		// set the viewmode to Third Person.
		SetViewMode(EALSViewMode::ThirdPerson);
	}

	if (CameraBehavior)
	{
		CameraBehavior->SetRotationMode(RotationMode);
	}

	MyCharacterMovementComponent->SetMovementSettings(GetTargetMovementSettings());
}

void AALSBaseCharacter::OnGaitChanged(const EALSGait PreviousGait)
{
	if (CameraBehavior)
	{
		CameraBehavior->Gait = Gait;
	}
}

void AALSBaseCharacter::OnViewModeChanged(const EALSViewMode PreviousViewMode)
{
	if (ViewMode == EALSViewMode::ThirdPerson)
	{
		if (RotationMode == EALSRotationMode::VelocityDirection || RotationMode == EALSRotationMode::LookingDirection)
		{
			// If Third Person, set the rotation mode back to the desired mode.
			SetRotationMode(DesiredRotationMode);
		}
	}
	else if (ViewMode == EALSViewMode::FirstPerson && RotationMode == EALSRotationMode::VelocityDirection)
	{
		// If First Person, set the rotation mode to looking direction if currently in the velocity direction mode.
		SetRotationMode(EALSRotationMode::LookingDirection);
	}

	if (CameraBehavior)
	{
		CameraBehavior->ViewMode = ViewMode;
	}
}

void AALSBaseCharacter::OnOverlayStateChanged(const EALSOverlayState PreviousState)
{
}

void AALSBaseCharacter::OnVisibleMeshChanged(const USkeletalMesh* PrevVisibleMesh)
{
	// Update the Skeletal Mesh before we update materials and anim bp variables
	GetMesh()->SetSkeletalMesh(VisibleMesh);

	// Reset materials to their new mesh defaults
	if (GetMesh() != nullptr)
	{
		for (int32 MaterialIndex = 0; MaterialIndex < GetMesh()->GetNumMaterials(); ++MaterialIndex)
		{
			GetMesh()->SetMaterial(MaterialIndex, nullptr);
		}
	}

	// Force set variables. This ensures anim instance & character stay synchronized on mesh changes
	ForceUpdateCharacterState();
}

void AALSBaseCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStance(EALSStance::Crouching);
}

void AALSBaseCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStance(EALSStance::Standing);
}

void AALSBaseCharacter::OnJumped_Implementation()
{
	Super::OnJumped_Implementation();
	if (IsLocallyControlled())
	{
		EventOnJumped();
	}
	if (HasAuthority())
	{
		Multicast_OnJumped();
	}
}

void AALSBaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (IsLocallyControlled())
	{
		EventOnLanded();
	}
	if (HasAuthority())
	{
		Multicast_OnLanded();
	}
}

void AALSBaseCharacter::OnLandFrictionReset()
{
	// Reset the braking friction
	GetCharacterMovement()->BrakingFrictionFactor = 0.0f;
}

void AALSBaseCharacter::SetEssentialValues(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::SetEssentialValues);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	if (GetLocalRole() != ROLE_SimulatedProxy)
	{
		if(GetCharacterMovement() != nullptr)
		{
			ReplicatedCurrentAcceleration = GetCharacterMovement()->GetCurrentAcceleration();
			EasedMaxAcceleration = GetCharacterMovement()->GetMaxAcceleration();
		}
		ReplicatedControlRotation = GetControlRotation();
	}
	else
	{
		if(GetCharacterMovement() != nullptr)
		{
			EasedMaxAcceleration = GetCharacterMovement()->GetMaxAcceleration() != 0
									   ? GetCharacterMovement()->GetMaxAcceleration()
									   : EasedMaxAcceleration / 2;
		}
	}

	// Interp AimingRotation to current control rotation for smooth character rotation movement. Decrease InterpSpeed
	// for slower but smoother movement.
	AimingRotation = FMath::RInterpTo(AimingRotation, ReplicatedControlRotation, DeltaTime, 30);

	// These values represent how the capsule is moving as well as how it wants to move, and therefore are essential
	// for any data driven animation system. They are also used throughout the system for various functions,
	// so I found it is easiest to manage them all in one place.

	const FVector CurrentVel = GetVelocity();

	// Set the amount of Acceleration.
	const FVector NewAcceleration = (CurrentVel - PreviousVelocity) / DeltaTime;
	Acceleration = NewAcceleration.IsNearlyZero() || IsLocallyControlled() ? NewAcceleration : Acceleration / 2;

	// Determine if the character is moving by getting it's speed. The Speed equals the length of the horizontal (x y)
	// velocity, so it does not take vertical movement into account. If the character is moving, update the last
	// velocity rotation. This value is saved because it might be useful to know the last orientation of movement
	// even after the character has stopped.
	Speed = CurrentVel.Size2D();
	bIsMoving = Speed > 1.0f;
	if (bIsMoving)
	{
		LastVelocityRotation = CurrentVel.ToOrientationRotator();
	}

	// Determine if the character has movement input by getting its movement input amount.
	// The Movement Input Amount is equal to the current acceleration divided by the max acceleration so that
	// it has a range of 0-1, 1 being the maximum possible amount of input, and 0 being none.
	// If the character has movement input, update the Last Movement Input Rotation.
	MovementInputAmount = ReplicatedCurrentAcceleration.Size() / EasedMaxAcceleration;
	bHasMovementInput = MovementInputAmount > 0.0f;
	if (bHasMovementInput)
	{
		LastMovementInputRotation = ReplicatedCurrentAcceleration.ToOrientationRotator();
	}

	// Set the Aim Yaw rate by comparing the current and previous Aim Yaw value, divided by Delta Seconds.
	// This represents the speed the camera is rotating left to right.
	AimYawRate = FMath::Abs((AimingRotation.Yaw - PreviousAimYaw) / DeltaTime);
}

void AALSBaseCharacter::UpdateCharacterMovement()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::UpdateCharacterMovement);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	// Set the Allowed Gait
	const EALSGait AllowedGait = GetAllowedGait();

	// Determine the Actual Gait. If it is different from the current Gait, Set the new Gait Event.
	const EALSGait ActualGait = GetActualGait(AllowedGait);

	if (ActualGait != Gait)
	{
		SetGait(ActualGait);
	}

	// Update the Character Max Walk Speed to the configured speeds based on the currently Allowed Gait.
	MyCharacterMovementComponent->SetAllowedGait(AllowedGait);
}
//@TODO check these for roation replication Bug//

void AALSBaseCharacter::UpdateGroundedRotation(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::UpdateGroundedRotation);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	if (MovementAction == EALSMovementAction::None)
	{
		const bool bCanUpdateMovingRot = ((bIsMoving && bHasMovementInput) || Speed > 150.0f) && !HasAnyRootMotion();
		if (bCanUpdateMovingRot)
		{
			const float GroundedRotationRate = CalculateGroundedRotationRate();
			if (RotationMode == EALSRotationMode::VelocityDirection)
			{
				// Velocity Direction Rotation
				SmoothCharacterRotation({0.0f, LastVelocityRotation.Yaw, 0.0f}, 800.0f, GroundedRotationRate,
				                        DeltaTime);
			}
			else if (RotationMode == EALSRotationMode::LookingDirection)
			{
				// Looking Direction Rotation
				float YawValue;
				if (Gait == EALSGait::Sprinting)
				{
					YawValue = LastVelocityRotation.Yaw;
				}
				else
				{
					// Walking or Running..
					const float YawOffsetCurveVal = GetAnimCurveValue(NAME_YawOffset);
					YawValue = AimingRotation.Yaw + YawOffsetCurveVal;
				}
				SmoothCharacterRotation({0.0f, YawValue, 0.0f}, 500.0f, GroundedRotationRate, DeltaTime);
			}
			else if (RotationMode == EALSRotationMode::Aiming)
			{
				const float ControlYaw = AimingRotation.Yaw;
				SmoothCharacterRotation({0.0f, ControlYaw, 0.0f}, 1000.0f, 20.0f, DeltaTime);
			}
		}
		else
		{
			// Not Moving

			if ((ViewMode == EALSViewMode::ThirdPerson && RotationMode == EALSRotationMode::Aiming) ||
				ViewMode == EALSViewMode::FirstPerson)
			{
				LimitRotation(-100.0f, 100.0f, 20.0f, DeltaTime);
			}

			// Apply the RotationAmount curve from Turn In Place Animations.
			// The Rotation Amount curve defines how much rotation should be applied each frame,
			// and is calculated for animations that are animated at 30fps.

			const float RotAmountCurve = GetAnimCurveValue(NAME_RotationAmount);

			if (FMath::Abs(RotAmountCurve) > 0.001f)
			{
				if (GetLocalRole() == ROLE_AutonomousProxy)
				{
					TargetRotation.Yaw = UKismetMathLibrary::NormalizeAxis(
						TargetRotation.Yaw + (RotAmountCurve * (DeltaTime / (1.0f / 30.0f))));
					SetActorRotation(TargetRotation);
				}
				else
				{
					AddActorWorldRotation({0, RotAmountCurve * (DeltaTime / (1.0f / 30.0f)), 0});
				}
				TargetRotation = GetActorRotation();
			}
		}
	}
	else if (MovementAction == EALSMovementAction::Rolling)
	{
		// Rolling Rotation (Not allowed on networked games)
		if (!bEnableNetworkOptimizations && bHasMovementInput)
		{
			SmoothCharacterRotation({0.0f, LastMovementInputRotation.Yaw, 0.0f}, 0.0f, 2.0f, DeltaTime);
		}
	}

	// Other actions are ignored...
}

void AALSBaseCharacter::UpdateInAirRotation(float DeltaTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSBaseCharacter::UpdateInAirRotation);
	SCOPE_CYCLE_COUNTER(STATGROUP_ALS_Base_Character);

	if (RotationMode == EALSRotationMode::VelocityDirection || RotationMode == EALSRotationMode::LookingDirection)
	{
		// Velocity / Looking Direction Rotation
		SmoothCharacterRotation({0.0f, InAirRotation.Yaw, 0.0f}, 0.0f, 5.0f, DeltaTime);
	}
	else if (RotationMode == EALSRotationMode::Aiming)
	{
		// Aiming Rotation
		SmoothCharacterRotation({0.0f, AimingRotation.Yaw, 0.0f}, 0.0f, 15.0f, DeltaTime);
		InAirRotation = GetActorRotation();
	}
}

EALSGait AALSBaseCharacter::GetAllowedGait() const
{
	// Calculate the Allowed Gait. This represents the maximum Gait the character is currently allowed to be in,
	// and can be determined by the desired gait, the rotation mode, the stance, etc. For example,
	// if you wanted to force the character into a walking state while indoors, this could be done here.

	if (Stance == EALSStance::Standing)
	{
		if (RotationMode != EALSRotationMode::Aiming)
		{
			if (DesiredGait == EALSGait::Sprinting)
			{
				return CanSprint() ? EALSGait::Sprinting : EALSGait::Running;
			}
			return DesiredGait;
		}
	}

	// Crouching stance & Aiming rot mode has same behaviour

	if (DesiredGait == EALSGait::Sprinting)
	{
		return EALSGait::Running;
	}

	return DesiredGait;
}

EALSGait AALSBaseCharacter::GetActualGait(EALSGait AllowedGait) const
{
	// Get the Actual Gait. This is calculated by the actual movement of the character,  and so it can be different
	// from the desired gait or allowed gait. For instance, if the Allowed Gait becomes walking,
	// the Actual gait will still be running until the character decelerates to the walking speed.

	const float LocWalkSpeed = MyCharacterMovementComponent->CurrentMovementSettings.WalkSpeed;
	const float LocRunSpeed = MyCharacterMovementComponent->CurrentMovementSettings.RunSpeed;

	if (Speed > LocRunSpeed + 10.0f)
	{
		if (AllowedGait == EALSGait::Sprinting)
		{
			return EALSGait::Sprinting;
		}
		return EALSGait::Running;
	}

	if (Speed >= LocWalkSpeed + 10.0f)
	{
		return EALSGait::Running;
	}

	return EALSGait::Walking;
}
//@TODO check these for roation replication Bug//
void AALSBaseCharacter::SmoothCharacterRotation(FRotator Target, float TargetInterpSpeed, float ActorInterpSpeed,
                                                float DeltaTime)
{
	// Interpolate the Target Rotation for extra smooth rotation behavior
	TargetRotation =
		FMath::RInterpConstantTo(TargetRotation, Target, DeltaTime, TargetInterpSpeed);
	SetActorRotation(
		FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, ActorInterpSpeed));
}

float AALSBaseCharacter::CalculateGroundedRotationRate() const
{
	// Calculate the rotation rate by using the current Rotation Rate Curve in the Movement Settings.
	// Using the curve in conjunction with the mapped speed gives you a high level of control over the rotation
	// rates for each speed. Increase the speed if the camera is rotating quickly for more responsive rotation.
	if (ensure(IsValid(MyCharacterMovementComponent) && ensure(IsValid(MyCharacterMovementComponent->CurrentMovementSettings.RotationRateCurve))))
	{
		const float MappedSpeedVal = MyCharacterMovementComponent->GetMappedSpeed();
		const float CurveVal =
			MyCharacterMovementComponent->CurrentMovementSettings.RotationRateCurve->GetFloatValue(MappedSpeedVal);
		const float ClampedAimYawRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 300.0f}, {1.0f, 3.0f}, AimYawRate);
		return CurveVal * ClampedAimYawRate;
	}
	return 0.0f;
}

void AALSBaseCharacter::LimitRotation(float AimYawMin, float AimYawMax, float InterpSpeed, float DeltaTime)
{
	// Prevent the character from rotating past a certain angle.
	FRotator Delta = AimingRotation - GetActorRotation();
	Delta.Normalize();
	const float RangeVal = Delta.Yaw;

	if (RangeVal < AimYawMin || RangeVal > AimYawMax)
	{
		const float ControlRotYaw = AimingRotation.Yaw;
		const float TargetYaw = ControlRotYaw + (RangeVal > 0.0f ? AimYawMin : AimYawMax);
		SmoothCharacterRotation({0.0f, TargetYaw, 0.0f}, 0.0f, InterpSpeed, DeltaTime);
	}
}

void AALSBaseCharacter::ForwardMovementAction(float Value)
{
	if (MovementState == EALSMovementState::Grounded || MovementState == EALSMovementState::InAir)
	{
		// Default camera relative movement behavior
		const FRotator DirRotator(0.0f, AimingRotation.Yaw, 0.0f);
		AddMovementInput(UKismetMathLibrary::GetForwardVector(DirRotator), Value);
	}
}

void AALSBaseCharacter::RightMovementAction(float Value)
{
	if (MovementState == EALSMovementState::Grounded || MovementState == EALSMovementState::InAir)
	{
		// Default camera relative movement behavior
		const FRotator DirRotator(0.0f, AimingRotation.Yaw, 0.0f);
		AddMovementInput(UKismetMathLibrary::GetRightVector(DirRotator), Value);
	}
}

void AALSBaseCharacter::CameraUpAction_Implementation(float Value)
{
	AddControllerPitchInput(LookUpDownRate * Value);
}

void AALSBaseCharacter::CameraRightAction_Implementation(float Value)
{
	AddControllerYawInput(LookLeftRightRate * Value);
}

void AALSBaseCharacter::JumpAction(bool bValue)
{
	if (bValue)
	{
		// Jump Action: Press "Jump Action" to end the ragdoll if ragdolling, stand up if crouching, or jump if standing.
		if (JumpPressedDelegate.IsBound())
		{
			JumpPressedDelegate.Broadcast();
		}

		if (MovementAction == EALSMovementAction::None)
		{
			if (MovementState == EALSMovementState::Grounded)
			{
				if (Stance == EALSStance::Standing)
				{
					Jump();
				}
				else if (Stance == EALSStance::Crouching)
				{
					UnCrouch();
				}
			}
			else if (MovementState == EALSMovementState::Ragdoll)
			{
				ReplicatedRagdollEnd();
			}
		}
	}
	else
	{
		StopJumping();
	}
}

void AALSBaseCharacter::SprintAction(bool bValue)
{
	if(bValue)
	{
		SetDesiredGait(EALSGait::Sprinting);
	}
	else
	{
		SetDesiredGait(EALSGait::Running);
	}
}

void AALSBaseCharacter::AimAction(bool bValue)
{
	if(bValue)
	{
		// AimAction: Hold "AimAction" to enter the aiming mode, release to revert back the desired rotation mode.
		SetRotationMode(EALSRotationMode::Aiming);
	}
	else
	{
		if (ViewMode == EALSViewMode::ThirdPerson)
		{
			SetRotationMode(DesiredRotationMode);
		}
		else if (ViewMode == EALSViewMode::FirstPerson)
		{
			SetRotationMode(EALSRotationMode::LookingDirection);
		}
	}
}

void AALSBaseCharacter::CameraTapAction()
{
	if (ViewMode == EALSViewMode::FirstPerson)
	{
		// Don't swap shoulders on first person mode
		return;
	}

	// Switch shoulders
	SetRightShoulder(!bRightShoulder);
}

void AALSBaseCharacter::CameraHeldAction()
{
	// Switch camera mode
	if (ViewMode == EALSViewMode::FirstPerson)
	{
		SetViewMode(EALSViewMode::ThirdPerson);
	}
	else if (ViewMode == EALSViewMode::ThirdPerson)
	{
		SetViewMode(EALSViewMode::FirstPerson);
	}
}

void AALSBaseCharacter::StanceAction()
{
	// Stance Action: Press "Stance Action" to toggle Standing / Crouching, double tap to Roll.

	if (MovementAction != EALSMovementAction::None)
	{
		return;
	}

	UWorld* World = GetWorld();
	check(World);

	const float PrevStanceInputTime = LastStanceInputTime;
	LastStanceInputTime = World->GetTimeSeconds();

	if (LastStanceInputTime - PrevStanceInputTime <= RollDoubleTapTimeout)
	{
		// Roll
		Replicated_PlayMontage(GetRollAnimation(), 1.15f);

		if (Stance == EALSStance::Standing)
		{
			SetDesiredStance(EALSStance::Crouching);
		}
		else if (Stance == EALSStance::Crouching)
		{
			SetDesiredStance(EALSStance::Standing);
		}
		return;
	}

	if (MovementState == EALSMovementState::Grounded)
	{
		if (Stance == EALSStance::Standing)
		{
			SetDesiredStance(EALSStance::Crouching);
			Crouch();
		}
		else if (Stance == EALSStance::Crouching)
		{
			SetDesiredStance(EALSStance::Standing);
			UnCrouch();
		}
	}

	// Notice: MovementState == EALSMovementState::InAir case is removed
}

void AALSBaseCharacter::WalkAction_Implementation()
{
	if (DesiredGait == EALSGait::Walking)
	{
		SetDesiredGait(EALSGait::Running);
	}
	else if (DesiredGait == EALSGait::Running)
	{
		SetDesiredGait(EALSGait::Walking);
	}
}

void AALSBaseCharacter::RagdollAction_Implementation()
{
	// Ragdoll Action: Press "Ragdoll Action" to toggle the ragdoll state on or off.

	if (GetMovementState() == EALSMovementState::Ragdoll)
	{
		ReplicatedRagdollEnd();
	}
	else
	{
		ReplicatedRagdollStart();
	}
}

void AALSBaseCharacter::VelocityDirectionAction_Implementation()
{
	// Select Rotation Mode: Switch the desired (default) rotation mode to Velocity or Looking Direction.
	// This will be the mode the character reverts back to when un-aiming
	SetDesiredRotationMode(EALSRotationMode::VelocityDirection);
	SetRotationMode(EALSRotationMode::VelocityDirection);
}

void AALSBaseCharacter::LookingDirectionAction_Implementation()
{
	SetDesiredRotationMode(EALSRotationMode::LookingDirection);
	SetRotationMode(EALSRotationMode::LookingDirection);
}

void AALSBaseCharacter::ReplicatedRagdollStart()
{
	if (HasAuthority())
	{
		Multicast_RagdollStart();
	}
	else
	{
		Server_RagdollStart();
	}
}

void AALSBaseCharacter::ReplicatedRagdollEnd()
{
	if (HasAuthority())
	{
		Multicast_RagdollEnd(GetActorLocation());
	}
	else
	{
		Server_RagdollEnd(GetActorLocation());
	}
}

void AALSBaseCharacter::OnRep_RotationMode(EALSRotationMode PrevRotMode)
{
	OnRotationModeChanged(PrevRotMode);
}

void AALSBaseCharacter::OnRep_ViewMode(EALSViewMode PrevViewMode)
{
	OnViewModeChanged(PrevViewMode);
}

void AALSBaseCharacter::OnRep_OverlayState(EALSOverlayState PrevOverlayState)
{
	OnOverlayStateChanged(PrevOverlayState);
}

void AALSBaseCharacter::OnRep_VisibleMesh(const USkeletalMesh* PreviousSkeletalMesh)
{
	OnVisibleMeshChanged(PreviousSkeletalMesh);
}

void AALSBaseCharacter::SetupCapsuleComponent() const
{
	if(GetCapsuleComponent() == nullptr){UE_LOG(LogTemp, Error, TEXT("AALSBaseCharacter::SetupCapsuleComponent FAILED"));return;}
	GetCapsuleComponent()->SetCapsuleHalfHeight(90.0f);
	GetCapsuleComponent()->SetCapsuleRadius(35.0f);
	GetCapsuleComponent()->SetAreaClassOverride(UNavArea_Obstacle::StaticClass());
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);	// Climable
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Ignore);	// WeaponTraceChannel
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel5, ECR_Overlap);	// Interactable
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel6, ECR_Ignore);	// Characters
}

void AALSBaseCharacter::SetupMeshComponent() const
{
	if(GetMesh() == nullptr){UE_LOG(LogTemp, Error, TEXT("AALSBaseCharacter::SetupMeshComponent FAILED"));return;}
	GetMesh()->SetWorldTransform(FTransform(FRotator(0.0f,-90.0f,0.0f), FVector(0.0f,0.0f,-92.0f)));
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel3, ECR_Ignore);	// Climable
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Block);		// WeaponTraceChannel
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel5, ECR_Block);		// Interactable
	GetMesh()->SetCollisionResponseToChannel(ECC_GameTraceChannel5, ECR_Block);		// Characters
	GetMesh()->SetGenerateOverlapEvents(true);
	//@TODO if Using only one Skeleton, Remove this	//
	GetMesh()->bUpdateMeshWhenKinematic = true;
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}
