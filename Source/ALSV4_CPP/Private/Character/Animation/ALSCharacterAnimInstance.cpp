// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community


#include "Character/Animation/ALSCharacterAnimInstance.h"
#include "Character/ALSBaseCharacter.h"
#include "Library/ALSMathLibrary.h"
#include "Components/ALSDebugComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Animation/AnimNode_StateMachine.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"


static const FName NAME_BasePose_CLF(TEXT("BasePose_CLF"));
static const FName NAME_BasePose_N(TEXT("BasePose_N"));
static const FName NAME_Enable_FootIK_R(TEXT("Enable_FootIK_R"));
static const FName NAME_Enable_FootIK_L(TEXT("Enable_FootIK_L"));
static const FName NAME_Enable_HandIK_L(TEXT("Enable_HandIK_L"));
static const FName NAME_Enable_HandIK_R(TEXT("Enable_HandIK_R"));
static const FName NAME_Enable_Transition(TEXT("Enable_Transition"));
static const FName NAME_FootLock_L(TEXT("FootLock_L"));
static const FName NAME_FootLock_R(TEXT("FootLock_R"));
static const FName NAME_Grounded___Slot(TEXT("Grounded Slot"));
static const FName NAME_Layering_Arm_L(TEXT("Layering_Arm_L"));
static const FName NAME_Layering_Arm_L_Add(TEXT("Layering_Arm_L_Add"));
static const FName NAME_Layering_Arm_L_LS(TEXT("Layering_Arm_L_LS"));
static const FName NAME_Layering_Arm_R(TEXT("Layering_Arm_R"));
static const FName NAME_Layering_Arm_R_Add(TEXT("Layering_Arm_R_Add"));
static const FName NAME_Layering_Arm_R_LS(TEXT("Layering_Arm_R_LS"));
static const FName NAME_Layering_Hand_L(TEXT("Layering_Hand_L"));
static const FName NAME_Layering_Hand_R(TEXT("Layering_Hand_R"));
static const FName NAME_Layering_Head_Add(TEXT("Layering_Head_Add"));
static const FName NAME_Layering_Spine_Add(TEXT("Layering_Spine_Add"));
static const FName NAME_Mask_AimOffset(TEXT("Mask_AimOffset"));
static const FName NAME_Mask_LandPrediction(TEXT("Mask_LandPrediction"));
static const FName NAME__ALSCharacterAnimInstance__RotationAmount(TEXT("RotationAmount"));
static const FName NAME_VB___foot_target_l(TEXT("VB foot_target_l"));
static const FName NAME_VB___foot_target_r(TEXT("VB foot_target_r"));
static const FName NAME_W_Gait(TEXT("W_Gait"));
static const FName NAME__ALSCharacterAnimInstance__root(TEXT("root"));

//~		Machines	 ~//

static const FName NAME__Machine__CLF_Directional_States(TEXT("CLF_Directional States"));
static const FName NAME__State__Move_RB(TEXT("Move RB"));
static const FName NAME__State__Move_LB(TEXT("Move LB"));
static const FName NAME__State__Move_LF(TEXT("Move LF"));
static const FName NAME__State__Move_RF(TEXT("Move RF"));

static const FName NAME__Machine__Look_Towards_Camera_States(TEXT("Look Towards Camera States"));
static const FName NAME__State__Looking_Left_and_Back(TEXT("Looking Left and Back"));
static const FName NAME__State__Looking_Right_and_Back(TEXT("Looking Right and Back"));
static const FName NAME__State__Looking_Forwards(TEXT("Looking Forwards"));

static const FName NAME__Machine__Jump_States(TEXT("Look Towards Camera States"));
static const FName NAME__State__Jump_Entry(TEXT("Entry"));
static const FName NAME__State__Jump_Left_Foot(TEXT("Jump Left Foot"));
static const FName NAME__State__Jump_Right_Foot(TEXT("Jump Right Foot"));
static const FName NAME__State__Jump_Loop(TEXT("Jump Loop"));
static const FName NAME__State__Flail(TEXT("Flail"));

static const FName NAME__Machine__N_Locomotion_Detail(TEXT("(N) Locomotion Detail"));
static const FName NAME__State__First_Pivot(TEXT("First Pivot"));
static const FName NAME__State__Second_Pivot(TEXT("Second Pivot"));
static const FName NAME__State__N_Run_Start(TEXT("(N) Run Start"));
static const FName NAME__State__N_Walk_Run(TEXT("(N) Walk->Run"));

static const FName NAME__Machine__CLF_Locomotion_States(TEXT("(CLF) Locomotion States"));
static const FName NAME__State__CLF_Moving(TEXT("(CLF) Moving"));
static const FName NAME__State__CLF_Stop(TEXT("(CLF) Stop"));

static const FName NAME__Machine__N_Locomotion_States(TEXT("(N) Locomotion States"));
static const FName NAME__State__N_Moving(TEXT("(N) Moving"));
static const FName NAME__State__N_Stop(TEXT("(N) Stop"));

static const FName NAME__Machine__Main_Grounded_States(TEXT("Main Grounded States"));
static const FName NAME__Machine__Main_Movement_States(TEXT("Main Movement States"));
static const FName NAME__State__Land(TEXT("Land"));

static const FName NAME__Machine__Rifle_States(TEXT("Rifle States"));

static const FName NAME__Machine__Pistol_1H_States(TEXT("Pistol 1H States"));
static const FName NAME__Machine__Pistol_2H_States(TEXT("Pistol 2H States"));

static const FName NAME__Machine__Bow_States(TEXT("Bow States"));

// MachineStateIndex_CLF_Directional_States = 63;
// StateIndex_Move_RB = 3;
// StateIndex_Move_LF = 4;
// MachineStateIndex_Look_Towards_Camera_States = 877;
// StateIndex_Looking_Left_and_Back = 1;
// StateIndex_Looking_Right_and_Back = 2;
// StateIndex_Looking_Forwards = 4;
//
// MachineStateIndex_N_Locomotion_States = 769;
// StateIndex_N_Moving = 1;
// StateIndex_N_Stop = 2;
//
// MachineStateIndex_CLF_Locomotion_States = 700;
// StateIndex_CLF_Moving = 1;
// StateIndex_CLF_Stop = 4;
//
// MachineStateIndex_N_Locomotion_Detail = 656;
// StateIndex_First_Pivot = 3;
// StateIndex_Second_Pivot = 4;
// StateIndex_N_Run_Start = 6;
// StateIndex_N_Walk_Run = 1;
//
// MachineStateIndex_Main_Grounded_States = 589;
// MachineStateIndex_Main_Movement_States = 558;
// StateIndex_Land = 3;
//
// MachineStateIndex_Jump_States = 540;
//
// MachineStateIndex_Rifle_States = 345;
//
// StateIndex_Entry = 0;
// StateIndex_Jump_Left_Foot = 1;
// StateIndex_Jump_Right_Foot = 2;
// StateIndex_Jump_Loop = 3;
// StateIndex_Flail = 4;


DECLARE_CYCLE_STAT(TEXT("ALS Animations (All Functions)"), STAT_ALS_Animations, STATGROUP_ALS);
DECLARE_CYCLE_STAT(TEXT("ALS Animations (SetValues)"), STAT_ALS_Animations_SetValues, STATGROUP_ALS);
// DECLARE_CYCLE_STAT(TEXT("ALS (All Functions)"), STAT_ALS_All, STAT_ALS);

// DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ALS NativeUpdateAnimation"), STAT_ALS_NativeUpdateAnimation, STAT_ALS)
// DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ALS ThreadSafeUpdateAnimation"), STAT_ALS_NativeThreadSafeUpdateAnimation, STAT_ALS)


UALSCharacterAnimInstance::UALSCharacterAnimInstance()
{
	FAnimStateMachineInfo DirectionalStates;
	DirectionalStates.Machine = FAnimStateInfo(NAME__Machine__CLF_Directional_States);
	DirectionalStates.States.Add(FAnimStateInfo(NAME__State__Move_RB));
	DirectionalStates.States.Add(FAnimStateInfo(NAME__State__Move_LB));
	DirectionalStates.States.Add(FAnimStateInfo(NAME__State__Move_LF));
	DirectionalStates.States.Add(FAnimStateInfo(NAME__State__Move_RF));
	StateMachineData.AddUnique(DirectionalStates);
	
	FAnimStateMachineInfo LookTowardsCameraStates;
	LookTowardsCameraStates.Machine = FAnimStateInfo(NAME__Machine__Look_Towards_Camera_States);
	LookTowardsCameraStates.States.Add(FAnimStateInfo(NAME__State__Looking_Left_and_Back));
	LookTowardsCameraStates.States.Add(FAnimStateInfo(NAME__State__Looking_Right_and_Back));
	LookTowardsCameraStates.States.Add(FAnimStateInfo(NAME__State__Looking_Forwards));
	StateMachineData.AddUnique(LookTowardsCameraStates);
	
	FAnimStateMachineInfo JumpStates;
	JumpStates.Machine = FAnimStateInfo(NAME__Machine__Jump_States);
	JumpStates.States.Add(FAnimStateInfo(NAME__State__Jump_Entry));
	JumpStates.States.Add(FAnimStateInfo(NAME__State__Jump_Left_Foot));
	JumpStates.States.Add(FAnimStateInfo(NAME__State__Jump_Right_Foot));
	JumpStates.States.Add(FAnimStateInfo(NAME__State__Jump_Loop));
	JumpStates.States.Add(FAnimStateInfo(NAME__State__Flail));
	StateMachineData.AddUnique(JumpStates);

	FAnimStateMachineInfo NLocomotionDetail;
	NLocomotionDetail.Machine = FAnimStateInfo(NAME__Machine__N_Locomotion_Detail);
	NLocomotionDetail.States.Add(FAnimStateInfo(NAME__State__First_Pivot));
	NLocomotionDetail.States.Add(FAnimStateInfo(NAME__State__Second_Pivot));
	NLocomotionDetail.States.Add(FAnimStateInfo(NAME__State__N_Run_Start));
	NLocomotionDetail.States.Add(FAnimStateInfo(NAME__State__N_Walk_Run));
	StateMachineData.AddUnique(NLocomotionDetail);

	FAnimStateMachineInfo CLFLocomotionDetail;
	CLFLocomotionDetail.Machine = FAnimStateInfo(NAME__Machine__CLF_Locomotion_States);
	CLFLocomotionDetail.States.Add(FAnimStateInfo(NAME__State__CLF_Moving));
	CLFLocomotionDetail.States.Add(FAnimStateInfo(NAME__State__CLF_Stop));
	StateMachineData.AddUnique(CLFLocomotionDetail);

	FAnimStateMachineInfo NLocomotionStates;
	NLocomotionStates.Machine = FAnimStateInfo(NAME__Machine__N_Locomotion_States);
	NLocomotionStates.States.Add(FAnimStateInfo(NAME__State__N_Moving));
	NLocomotionStates.States.Add(FAnimStateInfo(NAME__State__N_Stop));
	StateMachineData.AddUnique(NLocomotionStates);

	FAnimStateMachineInfo GroundedStates;
	GroundedStates.Machine = FAnimStateInfo(NAME__Machine__Main_Grounded_States);
	StateMachineData.AddUnique(GroundedStates);

	FAnimStateMachineInfo MovementStates;
	MovementStates.Machine = FAnimStateInfo(NAME__Machine__Main_Movement_States);
	MovementStates.States.Add(FAnimStateInfo(NAME__State__Land));
	StateMachineData.AddUnique(MovementStates);

	FAnimStateMachineInfo RifleStates;
	RifleStates.Machine = FAnimStateInfo(NAME__Machine__Rifle_States);
	StateMachineData.AddUnique(RifleStates);

	FAnimStateMachineInfo Pistol1HStates;
	Pistol1HStates.Machine = FAnimStateInfo(NAME__Machine__Pistol_1H_States);
	StateMachineData.AddUnique(Pistol1HStates);

	FAnimStateMachineInfo Pistol2HStates;
	Pistol2HStates.Machine = FAnimStateInfo(NAME__Machine__Pistol_2H_States);
	StateMachineData.AddUnique(Pistol2HStates);

	FAnimStateMachineInfo BowStates;
	BowStates.Machine = FAnimStateInfo(NAME__Machine__Bow_States);
	StateMachineData.AddUnique(BowStates);	
}

void UALSCharacterAnimInstance::NativeInitializeAnimation()
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::NativeInitializeAnimation);
	Super::NativeInitializeAnimation();
	Character = Cast<AALSBaseCharacter>(TryGetPawnOwner());
	if(Character)
	{
		GetStateMachineIndexes();
		Character->OnJumpedDelegate.AddUniqueDynamic(this, &UALSCharacterAnimInstance::OnJumped);
	}
}

void UALSCharacterAnimInstance::NativeBeginPlay()
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::NativeBeginPlay);
	// SCOPE_CYCLE_COUNTER(STAT_ALS_All);

	// it seems to be that the player pawn components are not really initialized
	// when the call to NativeInitializeAnimation() happens.
	// This is the reason why it is tried here to get the ALS debug component.
	if (const APawn* Owner = TryGetPawnOwner())
	{
		ALSDebugComponent = Owner->FindComponentByClass<UALSDebugComponent>();
	}

}

void UALSCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ALS NativeUpdateAnimation"), STAT_ALS_NativeUpdateAnimation, STATGROUP_ALS)
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::NativeUpdateAnimation);
	// SCOPE_CYCLE_COUNTER(STAT_ALS_All);
	Super::NativeUpdateAnimation(DeltaSeconds);
	if(bStopNativeUpdateAnimation){return;}

	if(!Character || DeltaSeconds == 0.0f)
	{
		return;
	}
	if(bOptimize)
	{
		UpdateFootIK(DeltaSeconds);
		if (MovementState.Grounded())
		{
			const bool bPrevShouldMove = Grounded.bShouldMove;
			Grounded.bShouldMove = ShouldMoveCheck();
			if (bPrevShouldMove == false && Grounded.bShouldMove)
			{
				// Do When Starting To Move
				TurnInPlaceValues.ElapsedDelayTime = 0.0f;
				Grounded.bRotateL = false;
				Grounded.bRotateR = false;
			}

			if (Grounded.bShouldMove)
			{
			}
			else
			{
				if (CanDynamicTransition())
				{
					DynamicTransitionCheck();
				}
			}

		}
	}
	else
	{
		// Update rest of character information. Others are reflected into anim bp when they're set inside character class
		CharacterInformation.MovementInputAmount = Character->GetMovementInputAmount();
		CharacterInformation.bHasMovementInput = Character->HasMovementInput();
		CharacterInformation.bIsMoving = Character->IsMoving();
		CharacterInformation.Acceleration = Character->GetAcceleration();
		CharacterInformation.AimYawRate = Character->GetAimYawRate();
		CharacterInformation.Speed = Character->GetSpeed();
		if(Character->GetCharacterMovement() != nullptr)
		{
			CharacterInformation.Velocity = Character->GetCharacterMovement()->Velocity;
		}
		CharacterInformation.MovementInput = Character->GetMovementInput();
		CharacterInformation.AimingRotation = Character->GetAimingRotation();
		CharacterInformation.CharacterActorRotation = Character->GetActorRotation();
		CharacterInformation.ViewMode = Character->GetViewMode();
		CharacterInformation.PrevMovementState = Character->GetPrevMovementState();
		LayerBlendingValues.OverlayOverrideState = Character->GetOverlayOverrideState();
        MovementState = Character->GetMovementState();
        MovementAction = Character->GetMovementAction();
        Stance = Character->GetStance();
        RotationMode = Character->GetRotationMode();
        Gait = Character->GetGait();
        OverlayState = Character->GetOverlayState();
        GroundedEntryState = Character->GetGroundedEntryState();


		UpdateAimingValues(DeltaSeconds);
		UpdateLayerValues();
		UpdateFootIK(DeltaSeconds);

		if (MovementState.Grounded())
		{
			// Check If Moving Or Not & Enable Movement Animations if IsMoving and HasMovementInput, or if the Speed is greater than 150.
			const bool bPrevShouldMove = Grounded.bShouldMove;
			Grounded.bShouldMove = ShouldMoveCheck();

			if (bPrevShouldMove == false && Grounded.bShouldMove)
			{
				// Do When Starting To Move
				TurnInPlaceValues.ElapsedDelayTime = 0.0f;
				Grounded.bRotateL = false;
				Grounded.bRotateR = false;
			}
			if (Grounded.bShouldMove)
			{
				// Do While Moving
				UpdateMovementValues(DeltaSeconds);
				UpdateRotationValues();
			}
			else
			{
				// Do While Not Moving
				if (CanRotateInPlace())
				{
					RotateInPlaceCheck();
				}
				else
				{
					Grounded.bRotateL = false;
					Grounded.bRotateR = false;
				}
				if (CanTurnInPlace())
				{
					TurnInPlaceCheck(DeltaSeconds);
				}
				else
				{
					TurnInPlaceValues.ElapsedDelayTime = 0.0f;
				}
				if (CanDynamicTransition())
				{
					DynamicTransitionCheck();
				}
			}
		}
		else if (MovementState.InAir())
		{
			// Do While InAir
			UpdateInAirValues(DeltaSeconds);
		}
		else if (MovementState.Ragdoll())
		{
			// Do While Ragdolling
			UpdateRagdollValues();
		}
	}

	
	// UpdateAimingValues(DeltaSeconds);
	// UpdateLayerValues();
	// UpdateFootIK(DeltaSeconds);

}

//@ Step 1 is to move all values to be set by interface
//@ Step 2 is to move any functions to the Thread safe update
//@ Step 3 Move all "setessential into" to only when it needs to be updated
//@ Add Seperate Function for Other Non Essential values

void UALSCharacterAnimInstance::SetEssentialInfo(const FALSAnimValues& Value)
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations_SetValues);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::SetEssentialInfo);

	CharacterInformation.MovementInputAmount = Value.CharacterInfo.MovementInputAmount;
	CharacterInformation.bHasMovementInput = Value.CharacterInfo.bHasMovementInput;
	CharacterInformation.bIsMoving = Value.CharacterInfo.bIsMoving;
	CharacterInformation.Acceleration = Value.CharacterInfo.Acceleration;
	CharacterInformation.AimYawRate = Value.CharacterInfo.AimYawRate;
	CharacterInformation.Speed = Value.CharacterInfo.Speed;
	CharacterInformation.MovementInput = Value.CharacterInfo.MovementInput;
	CharacterInformation.AimingRotation = Value.CharacterInfo.AimingRotation;
	CharacterInformation.CharacterActorRotation = Value.ActorRotation;
	CharacterInformation.PrevMovementState = Value.CharacterInfo.PrevMovementState;
	CharacterInformation.Velocity = Value.CharacterInfo.Velocity;
	// MovementState = Value.MovementState;
	MovementAction = Value.MovementAction;
}

void UALSCharacterAnimInstance::SetOptimize(const bool bValue)
{
	bOptimize = bValue;
}

void UALSCharacterAnimInstance::SetMovementState(const FALSMovementState& Value)
{
	MovementState = Value;
}

void UALSCharacterAnimInstance::SetOverlayOverrideState(const int32 Value)
{
	LayerBlendingValues.OverlayOverrideState = Value;
}

void UALSCharacterAnimInstance::SetGait(const FALSGait& Value)
{
	Gait = Value;
}

void UALSCharacterAnimInstance::SetStance(const FALSStance& Value)
{
	Stance = Value;
}

void UALSCharacterAnimInstance::SetViewMode(const EALSViewMode Value)
{
	CharacterInformation.ViewMode = Value;
}

void UALSCharacterAnimInstance::SetOverlayState(const FALSOverlayState& Value)
{
	OverlayState = Value;
}

void UALSCharacterAnimInstance::SetNewGroundedEntryState(const FALSGroundedEntryState& Value)
{
	GroundedEntryState = Value;
}

void UALSCharacterAnimInstance::SetRotationMode(const FALSRotationMode Value)
{
	RotationMode = Value;
}

void UALSCharacterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ALS ThreadSafeUpdateAnimation"), STAT_ALS_NativeThreadSafeUpdateAnimation, STATGROUP_ALS)
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::NativeThreadSafeUpdateAnimation);
	if(bStopNativeThreadSafeUpdateAnimation){return;}
	if(bOptimize)
	{
		UpdateAimingValues(DeltaSeconds);
		UpdateLayerValues();
		
		if (MovementState.Grounded())
		{
			// Check If Moving Or Not & Enable Movement Animations if IsMoving and HasMovementInput, or if the Speed is greater than 150.
			const bool bPrevShouldMove = Grounded.bShouldMove;
			Grounded.bShouldMove = ShouldMoveCheck();

			if (bPrevShouldMove == false && Grounded.bShouldMove)
			{
				// Do When Starting To Move
				TurnInPlaceValues.ElapsedDelayTime = 0.0f;
				Grounded.bRotateL = false;
				Grounded.bRotateR = false;
			}

			if (Grounded.bShouldMove)
			{
				// Do While Moving
				UpdateMovementValues(DeltaSeconds);
				UpdateRotationValues();
			}
			else
			{
				// Do While Not Moving
				if (CanRotateInPlace())
				{
					RotateInPlaceCheck();
				}
				else
				{
					Grounded.bRotateL = false;
					Grounded.bRotateR = false;
				}
				if (CanTurnInPlace())
				{
					TurnInPlaceCheck(DeltaSeconds);
				}
				else
				{
					TurnInPlaceValues.ElapsedDelayTime = 0.0f;
				}
				// if (CanDynamicTransition())
				// {
				// 	DynamicTransitionCheck();
				// }
			}
		}
		else if (MovementState.InAir())
		{
			// Do While InAir
			UpdateInAirValues(DeltaSeconds);
		}
		else if (MovementState.Ragdoll())
		{
			// Do While Ragdolling
			UpdateRagdollValues();
		}


	}
	// UpdateFootIK(DeltaSeconds);

}

void UALSCharacterAnimInstance::PlayTransition(const FALSDynamicMontageParams& Parameters)
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
    TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::PlayTransition);
	PlaySlotAnimationAsDynamicMontage(Parameters.Animation, NAME_Grounded___Slot,
	                                  Parameters.BlendInTime, Parameters.BlendOutTime, Parameters.PlayRate, 1,
	                                  0.0f, Parameters.StartTime);
}

void UALSCharacterAnimInstance::PlayTransitionChecked(const FALSDynamicMontageParams& Parameters)
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::PlayTransitionChecked);
	if (Stance.Standing() && !Grounded.bShouldMove)
	{
		PlayTransition(Parameters);
	}
}

void UALSCharacterAnimInstance::PlayDynamicTransition(float ReTriggerDelay, FALSDynamicMontageParams Parameters)
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::PlayDynamicTransition);
	if (bCanPlayDynamicTransition)
	{
		bCanPlayDynamicTransition = false;

		// Play Dynamic Additive Transition Animation
		PlayTransition(Parameters);

		UWorld* World = GetWorld();
		check(World);
		World->GetTimerManager().SetTimer(PlayDynamicTransitionTimer, this,
		                                  &UALSCharacterAnimInstance::PlayDynamicTransitionDelay,
		                                  ReTriggerDelay, false);
	}
}


bool UALSCharacterAnimInstance::GetShouldOverlayStateUsePRASIK() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetShouldOverlayStateUsePRASIK);
	switch (OverlayState) {
	case EALSOverlayState::Default:
	case EALSOverlayState::Masculine:
	case EALSOverlayState::Feminine:
	case EALSOverlayState::Injured:
	case EALSOverlayState::HandsTied:
	case EALSOverlayState::Torch:
	case EALSOverlayState::Binoculars:
	case EALSOverlayState::Box:
	case EALSOverlayState::Barrel:
		return false;
	case EALSOverlayState::Rifle:
	case EALSOverlayState::PistolOneHanded:
	case EALSOverlayState::PistolTwoHanded:
	case EALSOverlayState::Shotgun:
	case EALSOverlayState::Sniper:
	case EALSOverlayState::Launcher:
	case EALSOverlayState::Sword:
	case EALSOverlayState::Knife:
	case EALSOverlayState::Chainsaw:
	case EALSOverlayState::SwordShield:
	case EALSOverlayState::Melee:
	case EALSOverlayState::Bow:
	default:
		return true;
	}
}

// bool UALSCharacterAnimInstance::GetCanMoveLFtoMoveLB() const
// {
// 	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
// 	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetCanMoveLFtoMoveLB);
// 	const float CurveValue = GetCurveValue(FName("HipOrientation_Bias"));
// 	const bool bCurveTrue = UKismetMathLibrary::Abs(CurveValue) < 0.5f;
// 	// GetStateMachineInstanceFromName()
// 	// GetInstanceStateWeight()
// 	// return (CharacterInformation.bIsMoving && CharacterInformation.bHasMovementInput) ||
// 	// 	CharacterInformation.Speed > 150.0f;
// }

void UALSCharacterAnimInstance::GetStateMachineIndexes()
{
	SCOPE_CYCLE_COUNTER(STAT_BaseHelpers_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UCharacterAnimInstance::GetStateMachineIndexes);
	if(bHasInitializedIndexes){return;}
	for(auto& SingleStateMachineData : StateMachineData)
	{
		if(SingleStateMachineData.GetName() == NAME_None){continue;}
		const FAnimNode_StateMachine* Machine = GetStateMachineInstanceFromName(SingleStateMachineData.GetName());
		if(Machine == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Machine '%s' not found"), *SingleStateMachineData.GetName().ToString());
			continue;
		}
		SingleStateMachineData.Machine.SetIndex(GetStateMachineIndex(SingleStateMachineData.GetName()));
		// SingleStateMachineData.Machine.SetIndex(Machine->GetNodeIndex());
		if(SingleStateMachineData.States.IsEmpty())
		{
			continue;
		}
		IAnimClassInterface* Interface = IAnimClassInterface::GetFromClass(GetClass());
		const FBakedAnimationStateMachine* Baked = GetMachineDescription(Interface, const_cast<FAnimNode_StateMachine*>(Machine));
		if(Baked == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Baked machine '%s' not found"), *SingleStateMachineData.GetName().ToString());
			continue;
		}
		for(auto& State : SingleStateMachineData.States)
		{
			if(State.Name == NAME_None){continue;}
			const int32 StateIndex = Baked->FindStateIndex(State.Name);
			if(StateIndex == INDEX_NONE)
			{
				UE_LOG(LogTemp, Warning, TEXT("State '%s' not found in machine '%s'"), *State.Name.ToString(), *SingleStateMachineData.GetName().ToString());
				continue;
			}
			State.SetIndex(StateIndex);
		}
	}
	bHasInitializedIndexes = true;
}

bool UALSCharacterAnimInstance::GetStateWeight(const FName& MachineName, const FName& StateName, float& OutWeight) const
{
	SCOPE_CYCLE_COUNTER(STAT_BaseHelpers_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UCharacterAnimInstance::GetStateWeight);
	if(!IsValid(this) || !GetWorld() || !GetSkelMeshComponent() || !GetWorld()->IsGameWorld())
	{
		return false;
	}
	for(const FAnimStateMachineInfo& MachineData : StateMachineData)
	{
		if(MachineData.Machine.bIndexSet == false){continue;}
		if(MachineData.Machine.Name != MachineName){continue;}
		for(const FAnimStateInfo& State : MachineData.States)
		{
			if(State.bIndexSet == false){continue;}
			if(State.Name == StateName)
			{
				OutWeight = const_cast<UALSCharacterAnimInstance*>(this)->GetInstanceStateWeight(MachineData.GetIndex(), State.GetIndex());
				return true;
			}
		}
	}
	return false;
}

bool UALSCharacterAnimInstance::GetAnimTimeRemaining(const FName& MachineName, const FName& StateName,
	float& OutTime) const
{
	SCOPE_CYCLE_COUNTER(STAT_BaseHelpers_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UCharacterAnimInstance::GetStateWeight);
	if(!IsValid(this) || !GetWorld() || !GetSkelMeshComponent() || !GetWorld()->IsGameWorld())
	{
		return false;
	}
	for(const FAnimStateMachineInfo& MachineData : StateMachineData)
	{
		if(MachineData.Machine.bIndexSet == false){continue;}
		// Validate machine index
		if(MachineData.Machine.GetIndex() < 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Invalid Machine Index for %s"), *MachineName.ToString());
			return false;
		}
		if(MachineData.Machine.Name != MachineName){continue;}
		// const int32 NumMachines = GetStateMachineIndex();
		for(const FAnimStateInfo& State : MachineData.States)
		{
			if(State.bIndexSet == false){continue;}
			// Validate state index
			if(State.GetIndex() < 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("Invalid State Index for %s in machine %s"), *StateName.ToString(), *MachineName.ToString());
				return false;
			}
			if(State.Name == StateName)
			{
				const FAnimNode_StateMachine* MachineNode = GetStateMachineInstance(MachineData.GetIndex());
				if(!MachineNode)
				{
					UE_LOG(LogTemp, Warning, TEXT("MachineNode is null at index %d"), MachineData.GetIndex());
					return false;
				}
                UALSCharacterAnimInstance* MutableThis = const_cast<UALSCharacterAnimInstance*>(this);
				OutTime = MutableThis->GetRelevantAnimTimeRemaining(MachineData.GetIndex(), State.GetIndex());
				return true;
			}
		}
	}
	return false;
}

bool UALSCharacterAnimInstance::GetMachineWeight(const FName& MachineName, float& OutWeight) const
{
	SCOPE_CYCLE_COUNTER(STAT_BaseHelpers_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UCharacterAnimInstance::GetMachineWeight);
	if(!IsValid(this) || !GetWorld() || !GetSkelMeshComponent() || !GetWorld()->IsGameWorld())
	{
		return false;
	}
	for(const FAnimStateMachineInfo& MachineData : StateMachineData)
	{
		if(MachineData.Machine.bIndexSet == false){continue;}
		if(MachineData.Machine.Name != MachineName){continue;}
		OutWeight = const_cast<UALSCharacterAnimInstance*>(this)->GetInstanceMachineWeight(MachineData.GetIndex());
		return true;
	}
	return false;
}

bool UALSCharacterAnimInstance::GetCurrentStateTime(const FName& MachineName, float& OutTime) const
{
	SCOPE_CYCLE_COUNTER(STAT_BaseHelpers_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UCharacterAnimInstance::GetCurrentStateTime);
	if(!IsValid(this) || !GetWorld() || !GetSkelMeshComponent() || !GetWorld()->IsGameWorld())
	{
		return false;
	}
	for(const FAnimStateMachineInfo& MachineData : StateMachineData)
	{
		if(MachineData.Machine.bIndexSet == false){continue;}
		if(MachineData.Machine.Name != MachineName){continue;}
		OutTime = const_cast<UALSCharacterAnimInstance*>(this)->GetInstanceCurrentStateElapsedTime(MachineData.GetIndex());
		return true;
	}
	return false;
}

bool UALSCharacterAnimInstance::GetHipOrientationBiasOverHalf() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetHipOrientationBiasOverHalf);
	const float CurveValue = GetCurveValue(FName("HipOrientation_Bias"));
	return UKismetMathLibrary::Abs(CurveValue) > 0.5f;
}

bool UALSCharacterAnimInstance::GetPreStopToFootUpRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetPreStopToFootUpRule);
	const float CurveValue = GetCurveValue(FName("Feet_Position"));
	return UKismetMathLibrary::Abs(CurveValue) < 0.5f;
}

bool UALSCharacterAnimInstance::GetPreStopToFootDownRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetPreStopToFootDownRule);
	const float CurveValue = GetCurveValue(FName("Feet_Position"));
	return UKismetMathLibrary::Abs(CurveValue) >= 0.5f;
}

bool UALSCharacterAnimInstance::GetFeetCrossing() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetFeetCrossing);
	const float CurveValue = GetCurveValue(FName("Feet_Crossing"));
	return CurveValue != 0.0f;
}

bool UALSCharacterAnimInstance::GetRightFootRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetRightFootRule);
	const float CurveValue = GetCurveValue(FName("Feet_Position"));
	return CurveValue > 0.0f;
}

bool UALSCharacterAnimInstance::GetLeftFootRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLeftFootRule);
	const float CurveValue = GetCurveValue(FName("Feet_Position"));
	return CurveValue < 0.0f;
}

bool UALSCharacterAnimInstance::GetHipsRightRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetHipsRightRule);
	const float CurveValue = GetCurveValue(FName("HipOrientation_Bias"));
	const bool bHips =  UKismetMathLibrary::Abs(CurveValue) > 0.5f;
	const float CurveValueFeet = GetCurveValue(FName("Feet_Crossing"));
	const bool bCurveTrue = CurveValueFeet == 0.0f;
	return bHips && bCurveTrue;
}

bool UALSCharacterAnimInstance::GetHipsLeftRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetHipsLeftRule);
	const float CurveValue = GetCurveValue(FName("HipOrientation_Bias"));
	const bool bHip =  UKismetMathLibrary::Abs(CurveValue) < -0.5f;
	const float CurveValueFeet = GetCurveValue(FName("Feet_Crossing"));
	const bool bCurveTrue = CurveValueFeet == 0.0f;
	return bHip && bCurveTrue;
}

bool UALSCharacterAnimInstance::GetLookTowardFRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLookTowardFRule);
	return UKismetMathLibrary::InRange_FloatFloat(SmoothedAimingAngle.X, -1.0f * SmoothedAimingAngleFMax, SmoothedAimingAngleFMax, true, true);
}

bool UALSCharacterAnimInstance::GetLookTowardRBRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLookTowardRBRule);
	return UKismetMathLibrary::InRange_FloatFloat(SmoothedAimingAngle.X, SmoothedAimingAngleRBMin, SmoothedAimingAngleRBMax, true, true);
}

bool UALSCharacterAnimInstance::GetLookTowardLBRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLookTowardLBRule);
	return UKismetMathLibrary::InRange_FloatFloat(SmoothedAimingAngle.X, -1.0f *SmoothedAimingAngleRBMax, -1.0f * SmoothedAimingAngleRBMin, true, true);
}

bool UALSCharacterAnimInstance::GetRunningToWalkingRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetRunningToWalkingRule);
	const bool bWalking = Gait.Walking();
	const float CurveValue = GetCurveValue(FName("W_Gait"));
	const bool bCurveTrue = CurveValue < 1.2f;
	return bWalking && bCurveTrue;
}

bool UALSCharacterAnimInstance::GetEntryToCrouchingLFRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetEntryToCrouchingLFRule);
	const float CurveValue = GetCurveValue(FName("BasePose_CLF"));
	return CurveValue >= 0.5f;
}

bool UALSCharacterAnimInstance::GetEntryToStandingRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetEntryToStandingRule);
	const float CurveValue = GetCurveValue(FName("BasePose_CLF"));
	return CurveValue < 0.5f;
}

bool UALSCharacterAnimInstance::GetInterruptTransitionRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetInterruptTransitionRule);
	return (Grounded.bShouldMove || Grounded.bRotateL || Grounded.bRotateR);
}

bool UALSCharacterAnimInstance::GetEntryToJumpRightFoot() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetEntryToJumpRightFoot);
	const float CurveValue = GetCurveValue(FName("Feet_Position"));
	return CurveValue >= 0.0f;
}

bool UALSCharacterAnimInstance::GetLandToGroundRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLandToGroundRule);
	return (MovementState.Grounded() == false) || (Stance.Standing() == false);
}

bool UALSCharacterAnimInstance::GetMoveLFToMoveLBRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetMoveLFToMoveLBRule);
	const float CurveValue = GetCurveValue(FName("HipOrientation_Bias"));
	const bool bCurveTrue =  UKismetMathLibrary::Abs(CurveValue) < 0.5f;
	const bool bFeetCrossing = GetCurveValue(FName("Feet_Crossing")) == 0.0f;
	float Weight;
	const bool bWeight = GetStateWeight(NAME__Machine__CLF_Directional_States, NAME__State__Move_LF, Weight) && Weight == 1.0f;
	// const bool bWeight = GetStateWeightByName(FName("CLF_Directional States"), FName("Move LF")) == 1.0f;
	return bCurveTrue && bWeight && bFeetCrossing;
}

bool UALSCharacterAnimInstance::GetMoveRBtoMoveRFRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetMoveRBtoMoveRFRule);
	const float CurveValue = GetCurveValue(FName("HipOrientation_Bias"));
	const bool bCurveTrue =  UKismetMathLibrary::Abs(CurveValue) < 0.5f;
	const bool bFeetCrossing = GetCurveValue(FName("Feet_Crossing")) == 0.0f;
	float Weight;
	const bool bWeight = GetStateWeight(NAME__Machine__CLF_Directional_States, NAME__State__Move_RB, Weight) && Weight == 1.0f;
	// const bool bWeight = GetStateWeightByName(FName("CLF_Directional States"), FName("Move RB")) == 1.0f;
	return bCurveTrue && bWeight && bFeetCrossing;
}

bool UALSCharacterAnimInstance::GetMoveLBtoMoveLFRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetMoveLBtoMoveLFRule);
	const float CurveValue = GetCurveValue(FName("HipOrientation_Bias"));
	const bool bCurveTrue =  UKismetMathLibrary::Abs(CurveValue) < 0.5f;
	const bool bFeetCrossing = GetCurveValue(FName("Feet_Crossing")) == 0.0f;
	float Weight;
	const bool bWeight = GetStateWeight(NAME__Machine__CLF_Directional_States, NAME__State__Move_LB, Weight) && Weight == 1.0f;
	return bCurveTrue && bWeight && bFeetCrossing;
}

bool UALSCharacterAnimInstance::GetLookingLeftAndBackToLookingForwardRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLookingLeftAndBackToLookingForwardRule);
	const bool bInRange = UKismetMathLibrary::InRange_FloatFloat(SmoothedAimingAngle.X, -1.0f *SmoothedAimingAngleFMax, SmoothedAimingAngleFMax, true, true);
	float Weight;
	const bool bWeight = GetStateWeight(NAME__Machine__Look_Towards_Camera_States, NAME__State__Looking_Left_and_Back, Weight) && Weight != 1.0f;
	// const bool bWeight = GetStateWeightByName(FName("Look Towards Camera States"), FName("Looking Left and Back")) != 1.0f;
	return bInRange && bWeight;
}

bool UALSCharacterAnimInstance::GetLookingRightAndBackToLookingForwardRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLookingRightAndBackToLookingForwardRule);
	const bool bInRange = UKismetMathLibrary::InRange_FloatFloat(SmoothedAimingAngle.X, -1.0f *SmoothedAimingAngleFMax, SmoothedAimingAngleFMax, true, true);
	float Weight;
	const bool bWeight = GetStateWeight(NAME__Machine__Look_Towards_Camera_States, NAME__State__Looking_Right_and_Back, Weight) && Weight != 1.0f;
	// const bool bWeight = GetStateWeightByName(FName("Look Towards Camera States"), FName("Looking Right and Back")) != 1.0f;
	return bInRange && bWeight;
}

bool UALSCharacterAnimInstance::GetLookingForwardsToLookingRightBackRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLookingForwardsToLookingRightBackRule);
	const bool bInRange = UKismetMathLibrary::InRange_FloatFloat(SmoothedAimingAngle.X, SmoothedAimingAngleRBMin, SmoothedAimingAngleRBMax, true, true);
	float Weight;
	const bool bWeight = GetStateWeight(NAME__Machine__Look_Towards_Camera_States, NAME__State__Looking_Forwards, Weight) && Weight != 1.0f;
	return bInRange && bWeight;
}

bool UALSCharacterAnimInstance::GetLookingForwardsToLookingLeftBackRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLookingForwardsToLookingLeftBackRule);
	const bool bInRange = UKismetMathLibrary::InRange_FloatFloat(SmoothedAimingAngle.X, -1.0f * SmoothedAimingAngleRBMax, -1.0f * SmoothedAimingAngleRBMin, true, true);
	float Weight;
	const bool bWeight = GetStateWeight(NAME__Machine__Look_Towards_Camera_States, NAME__State__Looking_Forwards, Weight) && Weight != 1.0f;
	return bInRange && bWeight;
}

bool UALSCharacterAnimInstance::GetLookingToCameraNoOffsetRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLookingToCameraNoOffsetRule);
	float Time;
	return (GetCurrentStateTime(NAME__Machine__Look_Towards_Camera_States, Time)) && Time > 2.0f;
	// return GetCurrentStateTimeByName(FName("Look Towards Camera States")) > 2.0f;
}

bool UALSCharacterAnimInstance::GetMovingToStopRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetMovingToStopRule);
	float Weight;
	const bool bWeight = GetStateWeight(NAME__Machine__N_Locomotion_States, NAME__State__N_Moving, Weight) && Weight == 1.0f;
	return (Grounded.bShouldMove == false) && bWeight;
}

bool UALSCharacterAnimInstance::GetStopToNotMovingRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetStopToNotMovingRule);
	float Weight;
	return GetStateWeight(NAME__Machine__N_Locomotion_States, NAME__State__N_Stop, Weight) && Weight == 1.0f;
}

bool UALSCharacterAnimInstance::GetCLFMovingToStopRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetCLFMovingToStopRule);
	float Weight;
	const bool bWeight = GetStateWeight(NAME__Machine__N_Locomotion_States, NAME__State__CLF_Moving, Weight) && Weight == 1.0f;
	return (Grounded.bShouldMove == false) && bWeight;
}

bool UALSCharacterAnimInstance::GetCLFStopToNotMovingRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetCLFStopToNotMovingRule);
	float Weight;
	return GetStateWeight(NAME__Machine__N_Locomotion_States, NAME__State__CLF_Stop, Weight) && Weight == 1.0f;
}

bool UALSCharacterAnimInstance::GetWalkingToRunningRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetWalkingToRunningRule);
	float Weight;
	const bool bWeight = GetMachineWeight(NAME__Machine__Main_Grounded_States, Weight) && Weight != 1.0f;
	return (Gait.Walking() == false) && bWeight;
}

bool UALSCharacterAnimInstance::GetWalkingToRunRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetWalkingToRunRule);
	float Weight = 0.0f;
	const bool bWeight = GetMachineWeight(NAME__Machine__Main_Grounded_States, Weight) && Weight == 1.0f;
	return (Gait.Walking() == false) && bWeight;
}

bool UALSCharacterAnimInstance::GetWalkRunToRunningRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetWalkRunToRunningRule);
	float Weight = 0.0f;
	return  GetAnimTimeRemaining(NAME__Machine__N_Locomotion_Detail, NAME__State__N_Walk_Run, Weight) && Weight == 0.0f;
	// return (Gait.Walking() == false) && bWeight;
}

bool UALSCharacterAnimInstance::GetFirstPivotToSecondPivotRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetFirstPivotToSecondPivotRule);
	float Weight = 0.0f;
	const bool bWeight = GetCurrentStateTime(NAME__Machine__N_Locomotion_States, Weight) && Weight > 0.1f;
	return Grounded.bPivot && bWeight;
}

bool UALSCharacterAnimInstance::GetPivotToNRunningRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetPivotToNRunningRule);
	float Weight = 0.0f;
	return GetAnimTimeRemaining(NAME__Machine__N_Locomotion_Detail, NAME__State__First_Pivot, Weight) && Weight == 0.0f;
}

bool UALSCharacterAnimInstance::GetSecondPivotToNRunningRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetSecondPivotToNRunningRule);
	float Weight = 0.0f;
	return GetAnimTimeRemaining(NAME__Machine__N_Locomotion_Detail, NAME__State__Second_Pivot, Weight) && Weight == 0.0f;
}

bool UALSCharacterAnimInstance::GetSecondPivotToFirstPivotRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetSecondPivotToFirstPivotRule);
	float Weight = 0.0f;
	const bool bWeight = GetCurrentStateTime(NAME__Machine__N_Locomotion_States, Weight) && Weight > 0.1f;
	return Grounded.bPivot && bWeight;
}

bool UALSCharacterAnimInstance::GetRunToNRunStartRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetRunToNRunStartRule);
	float Weight = 0.0f;
	return GetMachineWeight(NAME__Machine__N_Locomotion_States, Weight) && Weight != 1.0f;
}

bool UALSCharacterAnimInstance::GetRunToNWalkRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetRunToNWalkRule);
	float Weight = 0.0f;
	return GetMachineWeight(NAME__Machine__N_Locomotion_States, Weight) && Weight == 1.0f;
}

bool UALSCharacterAnimInstance::GetRunStartToNRunning() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetRunStartToNRunning);
	float Weight;
	return GetAnimTimeRemaining(NAME__Machine__N_Locomotion_Detail, NAME__State__N_Run_Start, Weight) && Weight == 0.0f;
}

bool UALSCharacterAnimInstance::GetJumpLeftFootToJumpLoopRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetJumpLeftFootToJumpLoopRule);
	float Weight;
	return GetAnimTimeRemaining(NAME__Machine__Main_Movement_States, NAME__State__Jump_Left_Foot, Weight) && Weight == 0.0f;
}

bool UALSCharacterAnimInstance::GetJumpRightFootToJumpLoopRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetJumpRightFootToJumpLoopRule);
	float Weight;
	return GetAnimTimeRemaining(NAME__Machine__Main_Movement_States, NAME__State__Jump_Right_Foot, Weight) && Weight == 0.0f;
}

bool UALSCharacterAnimInstance::GetLandToGroundedRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLandToGroundedRule);
	return (Stance.Standing() == false) || (MovementState.Grounded() == false);
}

bool UALSCharacterAnimInstance::GetLandToLandMovementRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLandToLandMovementRule);
	return (Grounded.bRotateL) || (Grounded.bRotateR) || (CharacterInformation.bHasMovementInput) || (CharacterInformation.Speed > 650.0f);
}

bool UALSCharacterAnimInstance::GetLandToGroundedOtherRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLandToGroundedOtherRule);
	float Weight;
	return GetAnimTimeRemaining(NAME__Machine__Main_Movement_States, NAME__State__Land, Weight) && Weight == 0.0f;
}

bool UALSCharacterAnimInstance::GetLandMovementToGroundedRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLandMovementToGroundedRule);
	return (Stance.Standing() == false) || (MovementState.Grounded() == false);
}

bool UALSCharacterAnimInstance::GetLandToLandMovement() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetLandToLandMovement);
	return (Grounded.bRotateL) || (Grounded.bRotateR) || (CharacterInformation.bHasMovementInput);
}

bool UALSCharacterAnimInstance::GetRifleReadyToRifleRelaxedRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetRifleReadyToRifleRelaxedRule);
	const float CurveValue = GetCurveValue(FName("Enable_Transition"));
	const bool bCurveTrue =  CurveValue >= 0.99;
	float Weight;
	const bool bWeight = GetCurrentStateTime(NAME__Machine__Rifle_States, Weight) && Weight > 3.0f;
	return bCurveTrue && bWeight;
}

bool UALSCharacterAnimInstance::GetRifleReadyToRifleRelaxedSecondRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetRifleReadyToRifleRelaxedSecondRule);
	float Weight;
	const bool bWeight = GetCurrentStateTime(NAME__Machine__Rifle_States, Weight) && Weight > 3.0f;
	return CharacterInformation.bIsMoving && bWeight;
}

bool UALSCharacterAnimInstance::GetInAirOrSprintingRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetInAirOtSprintingRule);
	return MovementState.InAir() || Gait.Sprinting();
}

bool UALSCharacterAnimInstance::GetPistolReadyToPistolRelaxedRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetPistolReadyToPistolRelaxedRule);
	const float CurveValue = GetCurveValue(FName("Enable_Transition"));
	const bool bCurveTrue =  CurveValue >= 0.99f;
	float Weight;
	const bool bWeight = GetCurrentStateTime(NAME__Machine__Pistol_1H_States, Weight) && Weight > 3.0f;
	return bCurveTrue && bWeight;
}

bool UALSCharacterAnimInstance::GetPistolReadyToPistolRelaxedSecondRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetPistolReadyToPistolRelaxedSecondRule);
	float Weight;
	const bool bWeight = GetCurrentStateTime(NAME__Machine__Pistol_1H_States, Weight) && Weight > 3.0f;
	return CharacterInformation.bIsMoving && bWeight;
}

bool UALSCharacterAnimInstance::GetPistol2HReadyToPistolRelaxedRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetPistol2HReadyToPistolRelaxedRule);
	const float CurveValue = GetCurveValue(FName("Enable_Transition"));
	const bool bCurveTrue =  CurveValue >= 0.99f;
	float Weight;
	const bool bWeight = GetCurrentStateTime(NAME__Machine__Pistol_2H_States, Weight) && Weight > 3.0f;
	return bCurveTrue && bWeight;
}

bool UALSCharacterAnimInstance::GetPistol2HReadyToPistolRelaxedSecondRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetPistolReadyToPistolRelaxedSecondRule);
	float Weight;
	const bool bWeight = GetCurrentStateTime(NAME__Machine__Pistol_2H_States, Weight) && Weight > 3.0f;
	return CharacterInformation.bIsMoving && bWeight;
}

bool UALSCharacterAnimInstance::GetBowReadyToBowRelaxedRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetBowReadyToBowRelaxedRule);
	const float CurveValue = GetCurveValue(FName("RotationAmount"));
	const bool bCurveTrue =  CurveValue <= 0.01f;
	float Weight;
	const bool bWeight = GetCurrentStateTime(NAME__Machine__Bow_States, Weight) && Weight > 3.0f;
	return  bCurveTrue && bWeight;
}

bool UALSCharacterAnimInstance::GetBowReadyToBowRelaxedSecondRule() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetBowReadyToBowRelaxedSecondRule);
	float Weight;
	const bool bWeight = GetCurrentStateTime(NAME__Machine__Bow_States, Weight) && Weight > 3.0f;
	return CharacterInformation.bIsMoving && bWeight;
}

bool UALSCharacterAnimInstance::GetAimingOrFiringWeapon() const
{
	return RotationMode.Aiming() || bFiringWeapon;
}

bool UALSCharacterAnimInstance::GetNotAimingAndFiringWeapon() const
{
	return RotationMode.Aiming() == false && bFiringWeapon == false;
}


float UALSCharacterAnimInstance::GetAirFallSpeedABS() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetAirFallSpeedABS);
	return UKismetMathLibrary::Abs(InAir.FallSpeed);
}

bool UALSCharacterAnimInstance::ShouldMoveCheck() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::ShouldMoveCheck);
	return (CharacterInformation.bIsMoving && CharacterInformation.bHasMovementInput) ||
		CharacterInformation.Speed > 150.0f;
}

bool UALSCharacterAnimInstance::CanRotateInPlace() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::CanRotateInPlace);
	return RotationMode.Aiming() ||
		CharacterInformation.ViewMode == EALSViewMode::FirstPerson;
}

bool UALSCharacterAnimInstance::CanTurnInPlace() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::CanTurnInPlace);
	return RotationMode.LookingDirection() &&
		CharacterInformation.ViewMode == EALSViewMode::ThirdPerson &&
		GetCurveValue(NAME_Enable_Transition) >= 0.99f;
}

bool UALSCharacterAnimInstance::CanDynamicTransition() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::CanDynamicTransition);
	return GetCurveValue(NAME_Enable_Transition) >= 0.99f;
}

void UALSCharacterAnimInstance::PlayDynamicTransitionDelay()
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::PlayDynamicTransitionDelay);
	bCanPlayDynamicTransition = true;
}

void UALSCharacterAnimInstance::OnJumpedDelay()
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::OnJumpedDelay);
	InAir.bJumped = false;
}

void UALSCharacterAnimInstance::OnPivotDelay()
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::OnPivotDelay);
	Grounded.bPivot = false;
}

void UALSCharacterAnimInstance::UpdateAimingValues(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::UpdateAimingValues);
	// Interp the Aiming Rotation value to achieve smooth aiming rotation changes.
	// Interpolating the rotation before calculating the angle ensures the value is not affected by changes
	// in actor rotation, allowing slow aiming rotation changes with fast actor rotation changes.

	AimingValues.SmoothedAimingRotation = FMath::RInterpTo(AimingValues.SmoothedAimingRotation,
	                                                       CharacterInformation.AimingRotation, DeltaSeconds,
	                                                       Config.SmoothedAimingRotationInterpSpeed);

	// Calculate the Aiming angle and Smoothed Aiming Angle by getting
	// the delta between the aiming rotation and the actor rotation.
	FRotator Delta = CharacterInformation.AimingRotation - CharacterInformation.CharacterActorRotation;
	Delta.Normalize();
	AimingValues.AimingAngle.X = Delta.Yaw;
	AimingValues.AimingAngle.Y = Delta.Pitch;

	Delta = AimingValues.SmoothedAimingRotation - CharacterInformation.CharacterActorRotation;
	Delta.Normalize();
	SmoothedAimingAngle.X = Delta.Yaw;
	SmoothedAimingAngle.Y = Delta.Pitch;

	if (!RotationMode.VelocityDirection())
	{
		// Clamp the Aiming Pitch Angle to a range of 1 to 0 for use in the vertical aim sweeps.
		AimingValues.AimSweepTime = FMath::GetMappedRangeValueClamped<float, float>({-90.0f, 90.0f}, {1.0f, 0.0f},
		                                                              AimingValues.AimingAngle.Y);

		// Use the Aiming Yaw Angle divided by the number of spine+pelvis bones to get the amount of spine rotation
		// needed to remain facing the camera direction.
		AimingValues.SpineRotation.Roll = 0.0f;
		AimingValues.SpineRotation.Pitch = 0.0f;
		AimingValues.SpineRotation.Yaw = AimingValues.AimingAngle.X / 4.0f;
	}
	else if (CharacterInformation.bHasMovementInput)
	{
		// Get the delta between the Movement Input rotation and Actor rotation and map it to a range of 0-1.
		// This value is used in the aim offset behavior to make the character look toward the Movement Input.
		Delta = CharacterInformation.MovementInput.ToOrientationRotator() - CharacterInformation.CharacterActorRotation;
		Delta.Normalize();
		const float InterpTarget = FMath::GetMappedRangeValueClamped<float, float>({-180.0f, 180.0f}, {0.0f, 1.0f}, Delta.Yaw);

		AimingValues.InputYawOffsetTime = FMath::FInterpTo(AimingValues.InputYawOffsetTime, InterpTarget,
		                                                   DeltaSeconds, Config.InputYawOffsetInterpSpeed);
	}

	// Separate the Aiming Yaw Angle into 3 separate Yaw Times. These 3 values are used in the Aim Offset behavior
	// to improve the blending of the aim offset when rotating completely around the character.
	// This allows you to keep the aiming responsive but still smoothly blend from left to right or right to left.
	AimingValues.LeftYawTime = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 180.0f}, {0.5f, 0.0f},
	                                                             FMath::Abs(SmoothedAimingAngle.X));
	AimingValues.RightYawTime = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 180.0f}, {0.5f, 1.0f},
	                                                              FMath::Abs(SmoothedAimingAngle.X));
	AimingValues.ForwardYawTime = FMath::GetMappedRangeValueClamped<float, float>({-180.0f, 180.0f}, {0.0f, 1.0f},
	                                                                SmoothedAimingAngle.X);
}

void UALSCharacterAnimInstance::UpdateLayerValues()
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::UpdateLayerValues);
	// Get the Aim Offset weight by getting the opposite of the Aim Offset Mask.
	LayerBlendingValues.EnableAimOffset = FMath::Lerp(1.0f, 0.0f, GetCurveValue(NAME_Mask_AimOffset));
	// Set the Base Pose weights
	LayerBlendingValues.BasePose_N = GetCurveValue(NAME_BasePose_N);
	LayerBlendingValues.BasePose_CLF = GetCurveValue(NAME_BasePose_CLF);
	// Set the Additive amount weights for each body part
	LayerBlendingValues.Spine_Add = GetCurveValue(NAME_Layering_Spine_Add);
	LayerBlendingValues.Head_Add = GetCurveValue(NAME_Layering_Head_Add);
	LayerBlendingValues.Arm_L_Add = GetCurveValue(NAME_Layering_Arm_L_Add);
	LayerBlendingValues.Arm_R_Add = GetCurveValue(NAME_Layering_Arm_R_Add);
	// Set the Hand Override weights
	LayerBlendingValues.Hand_R = GetCurveValue(NAME_Layering_Hand_R);
	LayerBlendingValues.Hand_L = GetCurveValue(NAME_Layering_Hand_L);
	// Blend and set the Hand IK weights to ensure they only are weighted if allowed by the Arm layers.
	LayerBlendingValues.EnableHandIK_L = FMath::Lerp(0.0f, GetCurveValue(NAME_Enable_HandIK_L),
	                                                 GetCurveValue(NAME_Layering_Arm_L));
	LayerBlendingValues.EnableHandIK_R = FMath::Lerp(0.0f, GetCurveValue(NAME_Enable_HandIK_R),
	                                                 GetCurveValue(NAME_Layering_Arm_R));
	// Set whether the arms should blend in mesh space or local space.
	// The Mesh space weight will always be 1 unless the Local Space (LS) curve is fully weighted.
	LayerBlendingValues.Arm_L_LS = GetCurveValue(NAME_Layering_Arm_L_LS);
	LayerBlendingValues.Arm_L_MS = static_cast<float>(1 - FMath::FloorToInt(LayerBlendingValues.Arm_L_LS));
	LayerBlendingValues.Arm_R_LS = GetCurveValue(NAME_Layering_Arm_R_LS);
	LayerBlendingValues.Arm_R_MS = static_cast<float>(1 - FMath::FloorToInt(LayerBlendingValues.Arm_R_LS));
}

void UALSCharacterAnimInstance::UpdateFootIK(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::UpdateFootIK);
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ALS UpdateFootIK"), STAT_ALS_UpdateFootIK, STATGROUP_ALS)
	FVector FootOffsetLTarget = FVector::ZeroVector;
	FVector FootOffsetRTarget = FVector::ZeroVector;

	// Update Foot Locking values.
	SetFootLocking(DeltaSeconds, NAME_Enable_FootIK_L, NAME_FootLock_L,
	               IkFootL_BoneName, FootIKValues.FootLock_L_Alpha, FootIKValues.UseFootLockCurve_L,
	               FootIKValues.FootLock_L_Location, FootIKValues.FootLock_L_Rotation);
	SetFootLocking(DeltaSeconds, NAME_Enable_FootIK_R, NAME_FootLock_R,
	               IkFootR_BoneName, FootIKValues.FootLock_R_Alpha, FootIKValues.UseFootLockCurve_R,
	               FootIKValues.FootLock_R_Location, FootIKValues.FootLock_R_Rotation);

	if (MovementState.InAir())
	{
		// Reset IK Offsets if In Air
		SetPelvisIKOffset(DeltaSeconds, FVector::ZeroVector, FVector::ZeroVector);
		ResetIKOffsets(DeltaSeconds);
	}
	else if (!MovementState.Ragdoll())
	{
		// Update all Foot Lock and Foot Offset values when not In Air
		SetFootOffsets(DeltaSeconds, NAME_Enable_FootIK_L, IkFootL_BoneName, NAME__ALSCharacterAnimInstance__root,
		               FootOffsetLTarget,
		               FootIKValues.FootOffset_L_Location, FootIKValues.FootOffset_L_Rotation);
		SetFootOffsets(DeltaSeconds, NAME_Enable_FootIK_R, IkFootR_BoneName, NAME__ALSCharacterAnimInstance__root,
		               FootOffsetRTarget,
		               FootIKValues.FootOffset_R_Location, FootIKValues.FootOffset_R_Rotation);
		SetPelvisIKOffset(DeltaSeconds, FootOffsetLTarget, FootOffsetRTarget);
	}
}

void UALSCharacterAnimInstance::SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve,
                                               FName IKFootBone, float& CurFootLockAlpha, bool& UseFootLockCurve,
                                               FVector& CurFootLockLoc, FRotator& CurFootLockRot) const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::SetFootLocking);
	if (GetCurveValue(EnableFootIKCurve) <= 0.0f)
	{
		return;
	}

	// Step 1: Set Local FootLock Curve value
	float FootLockCurveVal;

	if (UseFootLockCurve)
	{
		UseFootLockCurve = FMath::Abs(GetCurveValue(NAME__ALSCharacterAnimInstance__RotationAmount)) <= 0.001f ||
			Character->GetLocalRole() != ROLE_AutonomousProxy;
		FootLockCurveVal = GetCurveValue(FootLockCurve) * (1.f / GetSkelMeshComponent()->AnimUpdateRateParams->UpdateRate);
	}
	else
	{
		UseFootLockCurve = GetCurveValue(FootLockCurve) >= 0.99f;
		FootLockCurveVal = 0.0f;
	}

	// Step 2: Only update the FootLock Alpha if the new value is less than the current, or it equals 1. This makes it
	// so that the foot can only blend out of the locked position or lock to a new position, and never blend in.
	if (FootLockCurveVal >= 0.99f || FootLockCurveVal < CurFootLockAlpha)
	{
		CurFootLockAlpha = FootLockCurveVal;
	}

	// Step 3: If the Foot Lock curve equals 1, save the new lock location and rotation in component space as the target.
	if (CurFootLockAlpha >= 0.99f)
	{
		const FTransform& OwnerTransform =
			GetOwningComponent()->GetSocketTransform(IKFootBone, RTS_Component);
		CurFootLockLoc = OwnerTransform.GetLocation();
		CurFootLockRot = OwnerTransform.Rotator();
	}

	// Step 4: If the Foot Lock Alpha has a weight,
	// update the Foot Lock offsets to keep the foot planted in place while the capsule moves.
	if (CurFootLockAlpha > 0.0f)
	{
		SetFootLockOffsets(DeltaSeconds, CurFootLockLoc, CurFootLockRot);
	}
}

void UALSCharacterAnimInstance::SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot) const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::SetFootLockOffsets);
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ALS SetFootLockOffsets"), STAT_ALS_SetFootLockOffsets, STATGROUP_ALS)

	FRotator RotationDifference = FRotator::ZeroRotator;
	// Use the delta between the current and last updated rotation to find how much the foot should be rotated
	// to remain planted on the ground.
	if (Character->GetCharacterMovement() != nullptr && Character->GetCharacterMovement()->IsMovingOnGround())
	{
		RotationDifference = CharacterInformation.CharacterActorRotation - Character->GetCharacterMovement()->
			GetLastUpdateRotation();
		RotationDifference.Normalize();
	}

	// Get the distance traveled between frames relative to the mesh rotation
	// to find how much the foot should be offset to remain planted on the ground.
	const FVector& LocationDifference = GetOwningComponent()->GetComponentRotation().UnrotateVector(
		CharacterInformation.Velocity * DeltaSeconds);

	// Subtract the location difference from the current local location and rotate
	// it by the rotation difference to keep the foot planted in component space.
	LocalLoc = (LocalLoc - LocationDifference).RotateAngleAxis(RotationDifference.Yaw, FVector::DownVector);

	// Subtract the Rotation Difference from the current Local Rotation to get the new local rotation.
	FRotator Delta = LocalRot - RotationDifference;
	Delta.Normalize();
	LocalRot = Delta;
}

void UALSCharacterAnimInstance::SetPelvisIKOffset(float DeltaSeconds, FVector FootOffsetLTarget,
                                                  FVector FootOffsetRTarget)
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::SetPelvisIKOffset);
	// Calculate the Pelvis Alpha by finding the average Foot IK weight. If the alpha is 0, clear the offset.
	FootIKValues.PelvisAlpha =
		(GetCurveValue(NAME_Enable_FootIK_L) + GetCurveValue(NAME_Enable_FootIK_R)) / 2.0f;

	if (FootIKValues.PelvisAlpha > 0.0f)
	{
		// Step 1: Set the new Pelvis Target to be the lowest Foot Offset
		const FVector PelvisTarget = FootOffsetLTarget.Z < FootOffsetRTarget.Z ? FootOffsetLTarget : FootOffsetRTarget;

		// Step 2: Interp the Current Pelvis Offset to the new target value.
		//Interpolate at different speeds based on whether the new target is above or below the current one.
		const float InterpSpeed = PelvisTarget.Z > FootIKValues.PelvisOffset.Z ? 10.0f : 15.0f;
		FootIKValues.PelvisOffset =
			FMath::VInterpTo(FootIKValues.PelvisOffset, PelvisTarget, DeltaSeconds, InterpSpeed);
	}
	else
	{
		FootIKValues.PelvisOffset = FVector::ZeroVector;
	}
}

void UALSCharacterAnimInstance::ResetIKOffsets(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::ResetIKOffsets);
	// Interp Foot IK offsets back to 0
	FootIKValues.FootOffset_L_Location = FMath::VInterpTo(FootIKValues.FootOffset_L_Location,
	                                                      FVector::ZeroVector, DeltaSeconds, 15.0f);
	FootIKValues.FootOffset_R_Location = FMath::VInterpTo(FootIKValues.FootOffset_R_Location,
	                                                      FVector::ZeroVector, DeltaSeconds, 15.0f);
	FootIKValues.FootOffset_L_Rotation = FMath::RInterpTo(FootIKValues.FootOffset_L_Rotation,
	                                                      FRotator::ZeroRotator, DeltaSeconds, 15.0f);
	FootIKValues.FootOffset_R_Rotation = FMath::RInterpTo(FootIKValues.FootOffset_R_Rotation,
	                                                      FRotator::ZeroRotator, DeltaSeconds, 15.0f);
}

void UALSCharacterAnimInstance::SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone,
                                               FName RootBone, FVector& CurLocationTarget, FVector& CurLocationOffset,
                                               FRotator& CurRotationOffset) const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::SetFootOffsets);
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ALS SetFootOffsets"), STAT_ALS_SetFootOffsets, STATGROUP_ALS)

	// Only update Foot IK offset values if the Foot IK curve has a weight. If it equals 0, clear the offset values.
	if (GetCurveValue(EnableFootIKCurve) <= 0)
	{
		CurLocationOffset = FVector::ZeroVector;
		CurRotationOffset = FRotator::ZeroRotator;
		return;
	}

	// Step 1: Trace downward from the foot location to find the geometry.
	// If the surface is walkable, save the Impact Location and Normal.
	USkeletalMeshComponent* OwnerComp = GetOwningComponent();
	FVector IKFootFloorLoc = OwnerComp->GetSocketLocation(IKFootBone);
	IKFootFloorLoc.Z = OwnerComp->GetSocketLocation(RootBone).Z;

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	const FVector TraceStart = IKFootFloorLoc + FVector(0.0, 0.0, Config.IK_TraceDistanceAboveFoot);
	const FVector TraceEnd = IKFootFloorLoc - FVector(0.0, 0.0, Config.IK_TraceDistanceBelowFoot);

	FHitResult HitResult;
	const bool bHit = World->LineTraceSingleByChannel(HitResult,
	                                                  TraceStart,
	                                                  TraceEnd,
	                                                  ECC_Visibility, Params);

	if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
	{
		UALSDebugComponent::DrawDebugLineTraceSingle(
			World,
			TraceStart,
			TraceEnd,
			EDrawDebugTrace::Type::ForOneFrame,
			bHit,
			HitResult,
			FLinearColor::Red,
			FLinearColor::Green,
			5.0f);
	}

	FRotator TargetRotOffset = FRotator::ZeroRotator;
	if (Character->GetCharacterMovement() != nullptr && Character->GetCharacterMovement()->IsWalkable(HitResult))
	{
		FVector ImpactPoint = HitResult.ImpactPoint;
		FVector ImpactNormal = HitResult.ImpactNormal;

		// Step 1.1: Find the difference in location from the Impact point and the expected (flat) floor location.
		// These values are offset by the normal multiplied by the
		// foot height to get better behavior on angled surfaces.
		CurLocationTarget = (ImpactPoint + ImpactNormal * Config.FootHeight) -
			(IKFootFloorLoc + FVector(0, 0, Config.FootHeight));

		// Step 1.2: Calculate the Rotation offset by getting the Atan2 of the Impact Normal.
		TargetRotOffset.Pitch = -FMath::RadiansToDegrees(FMath::Atan2(ImpactNormal.X, ImpactNormal.Z));
		TargetRotOffset.Roll = FMath::RadiansToDegrees(FMath::Atan2(ImpactNormal.Y, ImpactNormal.Z));
	}

	// Step 2: Interp the Current Location Offset to the new target value.
	// Interpolate at different speeds based on whether the new target is above or below the current one.
	const float InterpSpeed = CurLocationOffset.Z > CurLocationTarget.Z ? 30.f : 15.0f;
	CurLocationOffset = FMath::VInterpTo(CurLocationOffset, CurLocationTarget, DeltaSeconds, InterpSpeed);

	// Step 3: Interp the Current Rotation Offset to the new target value.
	CurRotationOffset = FMath::RInterpTo(CurRotationOffset, TargetRotOffset, DeltaSeconds, 30.0f);
}

void UALSCharacterAnimInstance::RotateInPlaceCheck()
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::RotateInPlaceCheck);
	// Step 1: Check if the character should rotate left or right by checking if the Aiming Angle exceeds the threshold.
	Grounded.bRotateL = AimingValues.AimingAngle.X < RotateInPlace.RotateMinThreshold;
	Grounded.bRotateR = AimingValues.AimingAngle.X > RotateInPlace.RotateMaxThreshold;

	// Step 2: If the character should be rotating, set the Rotate Rate to scale with the Aim Yaw Rate.
	// This makes the character rotate faster when moving the camera faster.
	if (Grounded.bRotateL || Grounded.bRotateR)
	{
		Grounded.RotateRate = FMath::GetMappedRangeValueClamped<float, float>(
			{RotateInPlace.AimYawRateMinRange, RotateInPlace.AimYawRateMaxRange},
			{RotateInPlace.MinPlayRate, RotateInPlace.MaxPlayRate},
			CharacterInformation.AimYawRate);
	}
}

void UALSCharacterAnimInstance::TurnInPlaceCheck(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::TurnInPlaceCheck);
	// Step 1: Check if Aiming angle is outside of the Turn Check Min Angle, and if the Aim Yaw Rate is below the Aim Yaw Rate Limit.
	// If so, begin counting the Elapsed Delay Time. If not, reset the Elapsed Delay Time.
	// This ensures the conditions remain true for a sustained period of time before turning in place.
	if (FMath::Abs(AimingValues.AimingAngle.X) <= TurnInPlaceValues.TurnCheckMinAngle ||
		CharacterInformation.AimYawRate >= TurnInPlaceValues.AimYawRateLimit)
	{
		TurnInPlaceValues.ElapsedDelayTime = 0.0f;
		return;
	}

	TurnInPlaceValues.ElapsedDelayTime += DeltaSeconds;
	const float ClampedAimAngle = FMath::GetMappedRangeValueClamped<float, float>({TurnInPlaceValues.TurnCheckMinAngle, 180.0f},
	                                                                {
		                                                                TurnInPlaceValues.MinAngleDelay,
		                                                                TurnInPlaceValues.MaxAngleDelay
	                                                                },
	                                                                AimingValues.AimingAngle.X);

	// Step 2: Check if the Elapsed Delay time exceeds the set delay (mapped to the turn angle range). If so, trigger a Turn In Place.
	if (TurnInPlaceValues.ElapsedDelayTime > ClampedAimAngle)
	{
		FRotator TurnInPlaceYawRot = CharacterInformation.AimingRotation;
		TurnInPlaceYawRot.Roll = 0.0f;
		TurnInPlaceYawRot.Pitch = 0.0f;
		TurnInPlace(TurnInPlaceYawRot, 1.0f, 0.0f, false);
	}
}

void UALSCharacterAnimInstance::DynamicTransitionCheck()
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::DynamicTransitionCheck);
	DECLARE_SCOPE_CYCLE_COUNTER(TEXT("ALS DynamicTransitionCheck"), STAT_ALS_DynamicTransitionCheck, STATGROUP_ALS);

	// Check each foot to see if the location difference between the IK_Foot bone and its desired / target location
	// (determined via a virtual bone) exceeds a threshold. If it does, play an additive transition animation on that foot.
	// The currently set transition plays the second half of a 2 foot transition animation, so that only a single foot moves.
	// Because only the IK_Foot bone can be locked, the separate virtual bone allows the system to know its desired location when locked.
	FTransform SocketTransformA = GetOwningComponent()->GetSocketTransform(IkFootL_BoneName, RTS_Component);
	FTransform SocketTransformB = GetOwningComponent()->GetSocketTransform(
		NAME_VB___foot_target_l, RTS_Component);
	float Distance = (SocketTransformB.GetLocation() - SocketTransformA.GetLocation()).Size();
	if (Distance > Config.DynamicTransitionThreshold)
	{
		FALSDynamicMontageParams Params;
		Params.Animation = TransitionAnim_R;
		Params.BlendInTime = 0.2f;
		Params.BlendOutTime = 0.2f;
		Params.PlayRate = 1.5f;
		Params.StartTime = 0.8f;
		PlayDynamicTransition(0.1f, Params);
	}

	SocketTransformA = GetOwningComponent()->GetSocketTransform(IkFootR_BoneName, RTS_Component);
	SocketTransformB = GetOwningComponent()->GetSocketTransform(NAME_VB___foot_target_r, RTS_Component);
	Distance = (SocketTransformB.GetLocation() - SocketTransformA.GetLocation()).Size();
	if (Distance > Config.DynamicTransitionThreshold)
	{
		FALSDynamicMontageParams Params;
		Params.Animation = TransitionAnim_L;
		Params.BlendInTime = 0.2f;
		Params.BlendOutTime = 0.2f;
		Params.PlayRate = 1.5f;
		Params.StartTime = 0.8f;
		PlayDynamicTransition(0.1f, Params);
	}
}

void UALSCharacterAnimInstance::UpdateMovementValues(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::UpdateMovementValues);
	// Interp and set the Velocity Blend.
	const FALSVelocityBlend& TargetBlend = CalculateVelocityBlend();
	VelocityBlend.F = FMath::FInterpTo(VelocityBlend.F, TargetBlend.F, DeltaSeconds, Config.VelocityBlendInterpSpeed);
	VelocityBlend.B = FMath::FInterpTo(VelocityBlend.B, TargetBlend.B, DeltaSeconds, Config.VelocityBlendInterpSpeed);
	VelocityBlend.L = FMath::FInterpTo(VelocityBlend.L, TargetBlend.L, DeltaSeconds, Config.VelocityBlendInterpSpeed);
	VelocityBlend.R = FMath::FInterpTo(VelocityBlend.R, TargetBlend.R, DeltaSeconds, Config.VelocityBlendInterpSpeed);

	// Set the Diagonal Scale Amount.
	Grounded.DiagonalScaleAmount = CalculateDiagonalScaleAmount();

	// Set the Relative Acceleration Amount and Interp the Lean Amount.
	RelativeAccelerationAmount = CalculateRelativeAccelerationAmount();
	LeanAmount.LR = FMath::FInterpTo(LeanAmount.LR, RelativeAccelerationAmount.Y, DeltaSeconds,
	                                 Config.GroundedLeanInterpSpeed);
	LeanAmount.FB = FMath::FInterpTo(LeanAmount.FB, RelativeAccelerationAmount.X, DeltaSeconds,
	                                 Config.GroundedLeanInterpSpeed);

	// Set the Walk Run Blend
	Grounded.WalkRunBlend = CalculateWalkRunBlend();

	// Set the Stride Blend
	Grounded.StrideBlend = CalculateStrideBlend();

	// Set the Standing and Crouching Play Rates
	Grounded.StandingPlayRate = CalculateStandingPlayRate();
	Grounded.CrouchingPlayRate = CalculateCrouchingPlayRate();
}

void UALSCharacterAnimInstance::UpdateRotationValues()
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::UpdateRotationValues);
	// Set the Movement Direction
	MovementDirection = CalculateMovementDirection();

	// Set the Yaw Offsets. These values influence the "YawOffset" curve in the AnimGraph and are used to offset
	// the characters rotation for more natural movement. The curves allow for fine control over how the offset
	// behaves for each movement direction.
	FRotator Delta = CharacterInformation.Velocity.ToOrientationRotator() - CharacterInformation.AimingRotation;
	Delta.Normalize();
	const FVector& FBOffset = YawOffset_FB->GetVectorValue(Delta.Yaw);
	Grounded.FYaw = FBOffset.X;
	Grounded.BYaw = FBOffset.Y;
	const FVector& LROffset = YawOffset_LR->GetVectorValue(Delta.Yaw);
	Grounded.LYaw = LROffset.X;
	Grounded.RYaw = LROffset.Y;
}

void UALSCharacterAnimInstance::UpdateInAirValues(float DeltaSeconds)
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::UpdateInAirValues);
	// Update the fall speed. Setting this value only while in the air allows you to use it within the AnimGraph for the landing strength.
	// If not, the Z velocity would return to 0 on landing.
	InAir.FallSpeed = CharacterInformation.Velocity.Z;

	// Set the Land Prediction weight.
	InAir.LandPrediction = CalculateLandPrediction();

	// Interp and set the In Air Lean Amount
	const FALSLeanAmount& InAirLeanAmount = CalculateAirLeanAmount();
	LeanAmount.LR = FMath::FInterpTo(LeanAmount.LR, InAirLeanAmount.LR, DeltaSeconds, Config.GroundedLeanInterpSpeed);
	LeanAmount.FB = FMath::FInterpTo(LeanAmount.FB, InAirLeanAmount.FB, DeltaSeconds, Config.GroundedLeanInterpSpeed);
}

void UALSCharacterAnimInstance::UpdateRagdollValues()
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::UpdateRagdollValues);
	// Scale the Flail Rate by the velocity length. The faster the ragdoll moves, the faster the character will flail.
	const float VelocityLength = GetOwningComponent()->GetPhysicsLinearVelocity(NAME__ALSCharacterAnimInstance__root).Size();
	FlailRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 1000.0f}, {0.0f, 1.0f}, VelocityLength);
}

float UALSCharacterAnimInstance::GetAnimCurveClamped(const FName& Name, float Bias, float ClampMin,
                                                     float ClampMax) const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::GetAnimCurveClamped);
	return FMath::Clamp(GetCurveValue(Name) + Bias, ClampMin, ClampMax);
}

FALSVelocityBlend UALSCharacterAnimInstance::CalculateVelocityBlend() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::CalculateVelocityBlend);
	// Calculate the Velocity Blend. This value represents the velocity amount of the actor in each direction (normalized so that
	// diagonals equal .5 for each direction), and is used in a BlendMulti node to produce better
	// directional blending than a standard blendspace.
	const FVector LocRelativeVelocityDir =
		CharacterInformation.CharacterActorRotation.UnrotateVector(CharacterInformation.Velocity.GetSafeNormal(0.1f));
	const float Sum = FMath::Abs(LocRelativeVelocityDir.X) + FMath::Abs(LocRelativeVelocityDir.Y) +
		FMath::Abs(LocRelativeVelocityDir.Z);
	const FVector RelativeDir = LocRelativeVelocityDir / Sum;
	FALSVelocityBlend Result;
	Result.F = FMath::Clamp(RelativeDir.X, 0.0f, 1.0f);
	Result.B = FMath::Abs(FMath::Clamp(RelativeDir.X, -1.0f, 0.0f));
	Result.L = FMath::Abs(FMath::Clamp(RelativeDir.Y, -1.0f, 0.0f));
	Result.R = FMath::Clamp(RelativeDir.Y, 0.0f, 1.0f);
	return Result;
}

FVector UALSCharacterAnimInstance::CalculateRelativeAccelerationAmount() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::CalculateRelativeAccelerationAmount);
	// Calculate the Relative Acceleration Amount. This value represents the current amount of acceleration / deceleration
	// relative to the actor rotation. It is normalized to a range of -1 to 1 so that -1 equals the Max Braking Deceleration,
	// and 1 equals the Max Acceleration of the Character Movement Component.
	if (FVector::DotProduct(CharacterInformation.Acceleration, CharacterInformation.Velocity) > 0.0f)
	{
		if(Character->GetCharacterMovement() != nullptr)
		{
			const float MaxAcc = Character->GetCharacterMovement()->GetMaxAcceleration();
			return CharacterInformation.CharacterActorRotation.UnrotateVector(
				CharacterInformation.Acceleration.GetClampedToMaxSize(MaxAcc) / MaxAcc);
		}
	}
	float MaxBrakingDec = 0.0f;
	if(Character->GetCharacterMovement() != nullptr)
	{
		MaxBrakingDec = Character->GetCharacterMovement()->GetMaxBrakingDeceleration();
	}
	return
		CharacterInformation.CharacterActorRotation.UnrotateVector(
			CharacterInformation.Acceleration.GetClampedToMaxSize(MaxBrakingDec) / MaxBrakingDec);
}

float UALSCharacterAnimInstance::CalculateStrideBlend() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::CalculateStrideBlend);
	// Calculate the Stride Blend. This value is used within the blendspaces to scale the stride (distance feet travel)
	// so that the character can walk or run at different movement speeds.
	// It also allows the walk or run gait animations to blend independently while still matching the animation speed to
	// the movement speed, preventing the character from needing to play a half walk+half run blend.
	// The curves are used to map the stride amount to the speed for maximum control.
	const float CurveTime = CharacterInformation.Speed / GetOwningComponent()->GetComponentScale().Z;
	const float ClampedGait = GetAnimCurveClamped(NAME_W_Gait, -1.0, 0.0f, 1.0f);
	const float LerpedStrideBlend =
		FMath::Lerp(StrideBlend_N_Walk->GetFloatValue(CurveTime), StrideBlend_N_Run->GetFloatValue(CurveTime),
		            ClampedGait);
	return FMath::Lerp(LerpedStrideBlend, StrideBlend_C_Walk->GetFloatValue(CharacterInformation.Speed),
	                   GetCurveValue(NAME_BasePose_CLF));
}

float UALSCharacterAnimInstance::CalculateWalkRunBlend() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::CalculateWalkRunBlend);
	// Calculate the Walk Run Blend. This value is used within the Blendspaces to blend between walking and running.
	return Gait.Walking() ? 0.0f : 1.0;
}

float UALSCharacterAnimInstance::CalculateStandingPlayRate() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::CalculateStandingPlayRate);
	// Calculate the Play Rate by dividing the Character's speed by the Animated Speed for each gait.
	// The lerps are determined by the "W_Gait" anim curve that exists on every locomotion cycle so
	// that the play rate is always in sync with the currently blended animation.
	// The value is also divided by the Stride Blend and the mesh scale so that the play rate increases as the stride or scale gets smaller
	const float LerpedSpeed = FMath::Lerp(CharacterInformation.Speed / Config.AnimatedWalkSpeed,
	                                      CharacterInformation.Speed / Config.AnimatedRunSpeed,
	                                      GetAnimCurveClamped(NAME_W_Gait, -1.0f, 0.0f, 1.0f));

	const float SprintAffectedSpeed = FMath::Lerp(LerpedSpeed, CharacterInformation.Speed / Config.AnimatedSprintSpeed,
	                                              GetAnimCurveClamped(NAME_W_Gait, -2.0f, 0.0f, 1.0f));

	return FMath::Clamp((SprintAffectedSpeed / Grounded.StrideBlend) / GetOwningComponent()->GetComponentScale().Z,
	                    0.0f, 3.0f);
}

float UALSCharacterAnimInstance::CalculateDiagonalScaleAmount() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::CalculateDiagonalScaleAmount);
	// Calculate the Diagonal Scale Amount. This value is used to scale the Foot IK Root bone to make the Foot IK bones
	// cover more distance on the diagonal blends. Without scaling, the feet would not move far enough on the diagonal
	// direction due to the linear translational blending of the IK bones. The curve is used to easily map the value.
	if (ensure(IsValid(DiagonalScaleAmountCurve)))
	{
		return DiagonalScaleAmountCurve->GetFloatValue(FMath::Abs(VelocityBlend.F + VelocityBlend.B));
	}
	return 0.0f;
}

float UALSCharacterAnimInstance::CalculateCrouchingPlayRate() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::CalculateCrouchingPlayRate);
	// Calculate the Crouching Play Rate by dividing the Character's speed by the Animated Speed.
	// This value needs to be separate from the standing play rate to improve the blend from crouch to stand while in motion.
	return FMath::Clamp(
		CharacterInformation.Speed / Config.AnimatedCrouchSpeed / Grounded.StrideBlend / GetOwningComponent()->
		GetComponentScale().Z,
		0.0f, 2.0f);
}

float UALSCharacterAnimInstance::CalculateLandPrediction() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::CalculateLandPrediction);
	// Calculate the land prediction weight by tracing in the velocity direction to find a walkable surface the character
	// is falling toward, and getting the 'Time' (range of 0-1, 1 being maximum, 0 being about to land) till impact.
	// The Land Prediction Curve is used to control how the time affects the final weight for a smooth blend.
	if (InAir.FallSpeed >= -200.0f)
	{
		return 0.0f;
	}

	const UCapsuleComponent* CapsuleComp = Character->GetCapsuleComponent();
	const FVector& CapsuleWorldLoc = CapsuleComp->GetComponentLocation();
	const float VelocityZ = CharacterInformation.Velocity.Z;
	FVector VelocityClamped = CharacterInformation.Velocity;
	VelocityClamped.Z = FMath::Clamp(VelocityZ, -4000.0f, -200.0f);
	VelocityClamped.Normalize();

	const FVector TraceLength = VelocityClamped * FMath::GetMappedRangeValueClamped<float, float>(
		{0.0f, -4000.0f}, {50.0f, 2000.0f}, VelocityZ);

	UWorld* World = GetWorld();
	check(World);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	FHitResult HitResult;
	const FCollisionShape CapsuleCollisionShape = FCollisionShape::MakeCapsule(CapsuleComp->GetUnscaledCapsuleRadius(),
	                                                                           CapsuleComp->GetUnscaledCapsuleHalfHeight());
	const bool bHit = World->SweepSingleByChannel(HitResult, CapsuleWorldLoc, CapsuleWorldLoc + TraceLength, FQuat::Identity,
	                                              ECC_Visibility, CapsuleCollisionShape, Params);

	if (ALSDebugComponent && ALSDebugComponent->GetShowTraces())
	{
		UALSDebugComponent::DrawDebugCapsuleTraceSingle(World,
		                                                CapsuleWorldLoc,
		                                                CapsuleWorldLoc + TraceLength,
		                                                CapsuleCollisionShape,
		                                                EDrawDebugTrace::Type::ForOneFrame,
		                                                bHit,
		                                                HitResult,
		                                                FLinearColor::Red,
		                                                FLinearColor::Green,
		                                                5.0f);
	}

	if (Character->GetCharacterMovement() != nullptr && Character->GetCharacterMovement()->IsWalkable(HitResult) && ensure(IsValid(LandPredictionCurve)))
	{
		return FMath::Lerp(LandPredictionCurve->GetFloatValue(HitResult.Time), 0.0f,GetCurveValue(NAME_Mask_LandPrediction));
	}

	return 0.0f;
}

FALSLeanAmount UALSCharacterAnimInstance::CalculateAirLeanAmount() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::CalculateAirLeanAmount);
	// Use the relative Velocity direction and amount to determine how much the character should lean while in air.
	// The Lean In Air curve gets the Fall Speed and is used as a multiplier to smoothly reverse the leaning direction
	// when transitioning from moving upwards to moving downwards.
	FALSLeanAmount CalcLeanAmount;
	if(ensure(LeanInAirCurve))
	{
		const FVector& UnrotatedVel = CharacterInformation.CharacterActorRotation.UnrotateVector(
			CharacterInformation.Velocity) / 350.0f;
		FVector2D InversedVect(UnrotatedVel.Y, UnrotatedVel.X);
		InversedVect *= LeanInAirCurve->GetFloatValue(InAir.FallSpeed);
		CalcLeanAmount.LR = InversedVect.X;
		CalcLeanAmount.FB = InversedVect.Y;
	}
	return CalcLeanAmount;
}

EALSMovementDirection UALSCharacterAnimInstance::CalculateMovementDirection() const
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::CalculateMovementDirection);
	// Calculate the Movement Direction. This value represents the direction the character is moving relative to the camera
	// during the Looking Direction / Aiming rotation modes, and is used in the Cycle Blending Anim Layers to blend to the
	// appropriate directional states.
	if (Gait.Sprinting() || RotationMode.VelocityDirection())
	{
		return EALSMovementDirection::Forward;
	}

	FRotator Delta = CharacterInformation.Velocity.ToOrientationRotator() - CharacterInformation.AimingRotation;
	Delta.Normalize();
	return UALSMathLibrary::CalculateQuadrant(MovementDirection, 70.0f, -70.0f, 110.0f, -110.0f, 5.0f, Delta.Yaw);
}

void UALSCharacterAnimInstance::TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime,
                                            bool OverrideCurrent)
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::TurnInPlace);
	// Step 1: Set Turn Angle
	FRotator Delta = TargetRotation - CharacterInformation.CharacterActorRotation;
	Delta.Normalize();
	const float TurnAngle = Delta.Yaw;

	// Step 2: Choose Turn Asset based on the Turn Angle and Stance
	FALSTurnInPlaceAsset TargetTurnAsset;
	if (Stance.Standing())
	{
		if (FMath::Abs(TurnAngle) < TurnInPlaceValues.Turn180Threshold)
		{
			TargetTurnAsset = TurnAngle < 0.0f
				                  ? TurnInPlaceValues.N_TurnIP_L90
				                  : TurnInPlaceValues.N_TurnIP_R90;
		}
		else
		{
			TargetTurnAsset = TurnAngle < 0.0f
				                  ? TurnInPlaceValues.N_TurnIP_L180
				                  : TurnInPlaceValues.N_TurnIP_R180;
		}
	}
	else
	{
		if (FMath::Abs(TurnAngle) < TurnInPlaceValues.Turn180Threshold)
		{
			TargetTurnAsset = TurnAngle < 0.0f
				                  ? TurnInPlaceValues.CLF_TurnIP_L90
				                  : TurnInPlaceValues.CLF_TurnIP_R90;
		}
		else
		{
			TargetTurnAsset = TurnAngle < 0.0f
				                  ? TurnInPlaceValues.CLF_TurnIP_L180
				                  : TurnInPlaceValues.CLF_TurnIP_R180;
		}
	}

	// Step 3: If the Target Turn Animation is not playing or set to be overriden, play the turn animation as a dynamic montage.
	if (!OverrideCurrent && IsPlayingSlotAnimation(TargetTurnAsset.Animation, TargetTurnAsset.SlotName))
	{
		return;
	}
	PlaySlotAnimationAsDynamicMontage(TargetTurnAsset.Animation, TargetTurnAsset.SlotName, 0.2f, 0.2f,
	                                  TargetTurnAsset.PlayRate * PlayRateScale, 1, 0.0f, StartTime);

	// Step 4: Scale the rotation amount (gets scaled in AnimGraph) to compensate for turn angle (If Allowed) and play rate.
	if (TargetTurnAsset.ScaleTurnAngle)
	{
		Grounded.RotationScale = (TurnAngle / TargetTurnAsset.AnimatedAngle) * TargetTurnAsset.PlayRate * PlayRateScale;
	}
	else
	{
		Grounded.RotationScale = TargetTurnAsset.PlayRate * PlayRateScale;
	}
}

void UALSCharacterAnimInstance::OnJumped()
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::OnJumped);
	InAir.bJumped = true;
	InAir.JumpPlayRate = FMath::GetMappedRangeValueClamped<float, float>({0.0f, 600.0f}, {1.2f, 1.5f}, CharacterInformation.Speed);

	GetWorld()->GetTimerManager().SetTimer(OnJumpedTimer, this,
	                                  &UALSCharacterAnimInstance::OnJumpedDelay, 0.1f, false);
}

void UALSCharacterAnimInstance::OnPivot()
{
	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::OnPivot);
	Grounded.bPivot = CharacterInformation.Speed < Config.TriggerPivotSpeedLimit;
	GetWorld()->GetTimerManager().SetTimer(OnPivotTimer, this,
	                                  &UALSCharacterAnimInstance::OnPivotDelay, 0.1f, false);
}

// void UALSCharacterAnimInstance::SetFiringWeapon(const bool bValue)
// {
// 	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
// 	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::SetFiringWeapon);
// 	bFiringWeapon = bValue;
// }
//
// void UALSCharacterAnimInstance::SetRecoilTransform(const FTransform& Transform)
// {
// 	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
// 	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::SetRecoilTransform);
// 	RecoilTransform.SetLocation(Transform.GetLocation());
// 	RecoilTransform.SetRotation(Transform.GetRotation());
// }
//
// void UALSCharacterAnimInstance::SetPivotPoint(const FTransform& Transform)
// {
// 	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
// 	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::SetPivotPoint);
// 	PivotPoint = Transform;
// }
//
// void UALSCharacterAnimInstance::DisableFootIK(const float DelayTime)
// {
// 	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
// 	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::DisableFootIK);
// 	OnDisableFootIK(DelayTime);
// }
//
// void UALSCharacterAnimInstance::SetInjured(const EBodyPartName BodyPartName, const float InjuredAmount)
// {
// 	SCOPE_CYCLE_COUNTER(STAT_ALS_Animations);
// 	TRACE_CPUPROFILER_EVENT_SCOPE(UALSCharacterAnimInstance::SetInjured);
// 	InjuredBodyPart = BodyPartName;
// 	InjuredAlpha = InjuredAmount;
// 	OnInjured(BodyPartName, InjuredAmount);
// }
