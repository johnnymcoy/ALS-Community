// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character/Animation/CharacterAnimInstance.h"
#include "Data/BaseData.h"
#include "Interfaces/ALSAnimInterface.h"
#include "Interfaces/CustomAnimInstance.h"
#include "Library/ALSAnimationStructLibrary.h"
#include "Library/ALSStructEnumLibrary.h"

#include "ALSCharacterAnimInstance.generated.h"

// forward declarations
class UALSDebugComponent;
class AALSBaseCharacter;
class UCurveFloat;
class UAnimSequence;
class UCurveVector;

/**
 * Main anim instance class for character
 */
UCLASS(Blueprintable, BlueprintType)
class ALSV4_CPP_API UALSCharacterAnimInstance : public UCharacterAnimInstance, public IALSAnimInterface
{
	GENERATED_BODY()

public:

	UALSCharacterAnimInstance();
	
	virtual void NativeInitializeAnimation() override;

	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "ALS|Animation")
	void PlayTransition(const FALSDynamicMontageParams& Parameters);

	UFUNCTION(BlueprintCallable, Category = "ALS|Animation")
	void PlayTransitionChecked(const FALSDynamicMontageParams& Parameters);

	UFUNCTION(BlueprintCallable, Category = "ALS|Animation")
	void PlayDynamicTransition(float ReTriggerDelay, FALSDynamicMontageParams Parameters);

	UFUNCTION(BlueprintCallable, Category = "ALS|Event")
	void OnJumped();

	UFUNCTION(BlueprintCallable, Category = "ALS|Event")
	void OnPivot();
	
	virtual void SetEssentialInfo(const FALSAnimValues& Value) override;
	virtual void SetOptimize(const bool bValue) override;
	virtual void SetMovementState(const FALSMovementState& Value) override;
	virtual void SetOverlayOverrideState(const int32 Value) override;
	virtual void SetGait(const FALSGait& Value) override;
	virtual void SetStance(const FALSStance& Value) override;
	virtual void SetViewMode(const EALSViewMode Value) override;
	virtual void SetOverlayState(const FALSOverlayState& Value) override;
	virtual void SetNewGroundedEntryState(const FALSGroundedEntryState& Value) override;
	virtual void SetRotationMode(const FALSRotationMode Value) override;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ALS|Debug")
	bool bOptimize = false;

	UFUNCTION(BlueprintCallable, Category="Custom ALS")
	FRotator GetRecoilRotation() const {return RecoilTransform.Rotator();};
	UFUNCTION(BlueprintCallable, Category="Custom ALS")
	FVector GetPivotPointLocation() const {return PivotPoint.GetLocation() + GunOffset;};
	UFUNCTION(BlueprintCallable, Category="Custom ALS")
	bool GetShouldOverlayStateUsePRASIK() const;

protected:
	/** Optimizations  */


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ALS|Debug")
	bool bStopNativeUpdateAnimation = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ALS|Debug")
	bool bStopNativeThreadSafeUpdateAnimation = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ALS|Optimization")
	TArray<FAnimStateMachineInfo> StateMachineData;
	UPROPERTY(Transient)
	bool bHasInitializedIndexes = false;
	
	void GetStateMachineIndexes();
	bool GetStateWeight(const FName& MachineName, const FName& StateName, float& OutWeight) const;
	bool GetAnimTimeRemaining(const FName& MachineName, const FName& StateName, float& OutTime) const;
	bool GetMachineWeight(const FName& MachineName, float& OutWeight) const;
	bool GetCurrentStateTime(const FName& MachineName, float& OutTime) const;

	//Figure out all indexes of Needed States and Weights etc. optimize  
	// Returns if the HipOrientation_Bias ABS < 0.5f
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetHipOrientationBiasOverHalf() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetPreStopToFootUpRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetPreStopToFootDownRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetFeetCrossing() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetRightFootRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLeftFootRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetHipsRightRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetHipsLeftRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLookTowardFRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLookTowardRBRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLookTowardLBRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetRunningToWalkingRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetEntryToCrouchingLFRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetEntryToStandingRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetInterruptTransitionRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetEntryToJumpRightFoot() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLandToGroundRule() const;

	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetMoveLFToMoveLBRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetMoveRBtoMoveRFRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetMoveLBtoMoveLFRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLookingLeftAndBackToLookingForwardRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLookingRightAndBackToLookingForwardRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLookingForwardsToLookingRightBackRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLookingForwardsToLookingLeftBackRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLookingToCameraNoOffsetRule() const;

	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetMovingToStopRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetStopToNotMovingRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetCLFMovingToStopRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetCLFStopToNotMovingRule() const;

	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetWalkingToRunningRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetWalkingToRunRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetWalkRunToRunningRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetFirstPivotToSecondPivotRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetPivotToNRunningRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetSecondPivotToNRunningRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetSecondPivotToFirstPivotRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetRunToNRunStartRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetRunToNWalkRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetRunStartToNRunning() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetJumpLeftFootToJumpLoopRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetJumpRightFootToJumpLoopRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLandToGroundedRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLandToLandMovementRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLandToGroundedOtherRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLandMovementToGroundedRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetLandToLandMovement() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetRifleReadyToRifleRelaxedRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetRifleReadyToRifleRelaxedSecondRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetInAirOrSprintingRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetPistolReadyToPistolRelaxedRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetPistolReadyToPistolRelaxedSecondRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetPistol2HReadyToPistolRelaxedRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetPistol2HReadyToPistolRelaxedSecondRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetBowReadyToBowRelaxedRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetBowReadyToBowRelaxedSecondRule() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetAimingOrFiringWeapon() const;
	UFUNCTION(BlueprintCallable, Category="ALS|Rules")
	bool GetNotAimingAndFiringWeapon() const;

	UFUNCTION(BlueprintCallable, Category="ALS|Rules", meta = (BlueprintThreadSafe))
	float GetAirFallSpeedABS() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ALS|Extras")
	float SmoothedAimingAngleFMax = 125.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ALS|Extras")
	float SmoothedAimingAngleRBMax = 180.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="ALS|Extras")
	float SmoothedAimingAngleRBMin = 130.0f;

protected:

	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	void SetTrackedHipsDirection(EALSHipsDirection HipsDirection)
	{
		Grounded.TrackedHipsDirection = HipsDirection;
	}

	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	void SetGroundedEntryState(EALSGroundedEntryState NewState)
	{
		GroundedEntryState = NewState;
	}

	/** Enable Movement Animations if IsMoving and HasMovementInput, or if the Speed is greater than 150. */
	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	bool ShouldMoveCheck() const;

	/** Only perform a Rotate In Place Check if the character is Aiming or in First Person. */
	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	bool CanRotateInPlace() const;

	/**
	 * Only perform a Turn In Place check if the character is looking toward the camera in Third Person,
	 * and if the "Enable Transition" curve is fully weighted. The Enable_Transition curve is modified within certain
	 * states of the AnimBP so that the character can only turn while in those states..
	 */
	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	bool CanTurnInPlace() const;

	/**
	 * Only perform a Dynamic Transition check if the "Enable Transition" curve is fully weighted.
	 * The Enable_Transition curve is modified within certain states of the AnimBP so
	 * that the character can only transition while in those states.
	 */
	UFUNCTION(BlueprintCallable, Category = "ALS|Grounded")
	bool CanDynamicTransition() const;

private:
	void PlayDynamicTransitionDelay();

	void OnJumpedDelay();

	void OnPivotDelay();

	/** Update Values */

	void UpdateAimingValues(float DeltaSeconds);

	void UpdateLayerValues();

	void UpdateFootIK(float DeltaSeconds);

	void UpdateMovementValues(float DeltaSeconds);

	void UpdateRotationValues();

	void UpdateInAirValues(float DeltaSeconds);

	void UpdateRagdollValues();

	/** Foot IK */

	void SetFootLocking(float DeltaSeconds, FName EnableFootIKCurve, FName FootLockCurve, FName IKFootBone,
                          float& CurFootLockAlpha, bool& UseFootLockCurve,
                          FVector& CurFootLockLoc, FRotator& CurFootLockRot) const;

	void SetFootLockOffsets(float DeltaSeconds, FVector& LocalLoc, FRotator& LocalRot) const;

	void SetPelvisIKOffset(float DeltaSeconds, FVector FootOffsetLTarget, FVector FootOffsetRTarget);

	void ResetIKOffsets(float DeltaSeconds);

	void SetFootOffsets(float DeltaSeconds, FName EnableFootIKCurve, FName IKFootBone, FName RootBone,
                          FVector& CurLocationTarget, FVector& CurLocationOffset, FRotator& CurRotationOffset) const;

	/** Grounded */

	void RotateInPlaceCheck();

	void TurnInPlaceCheck(float DeltaSeconds);

	void DynamicTransitionCheck();

	FALSVelocityBlend CalculateVelocityBlend() const;

	void TurnInPlace(FRotator TargetRotation, float PlayRateScale, float StartTime, bool OverrideCurrent);

	/** Movement */

	FVector CalculateRelativeAccelerationAmount() const;

	float CalculateStrideBlend() const;

	float CalculateWalkRunBlend() const;

	float CalculateStandingPlayRate() const;

	float CalculateDiagonalScaleAmount() const;

	float CalculateCrouchingPlayRate() const;

	float CalculateLandPrediction() const;

	FALSLeanAmount CalculateAirLeanAmount() const;

	EALSMovementDirection CalculateMovementDirection() const;

	/** Util */

	float GetAnimCurveClamped(const FName& Name, float Bias, float ClampMin, float ClampMax) const;

public:
	/** References */
	UPROPERTY(BlueprintReadOnly, Category = "Read Only Data|Character Information")
	TObjectPtr<AALSBaseCharacter> Character = nullptr;

	/** Character Information */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimCharacterInformation CharacterInformation;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FALSMovementState MovementState = EALSMovementState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FALSMovementAction MovementAction = EALSMovementAction::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FALSRotationMode RotationMode = EALSRotationMode::VelocityDirection;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FALSGait Gait = EALSGait::Walking;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FALSStance Stance = EALSStance::Standing;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Character Information")
	FALSOverlayState OverlayState = EALSOverlayState::Default;

	/** Anim Graph - Grounded */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimGraphGrounded Grounded;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FALSVelocityBlend VelocityBlend;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FALSLeanAmount LeanAmount;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FVector RelativeAccelerationAmount = FVector::ZeroVector;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FALSGroundedEntryState GroundedEntryState = EALSGroundedEntryState::None;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Grounded")
	FALSMovementDirection MovementDirection = EALSMovementDirection::Forward;

	/** Anim Graph - In Air */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - In Air", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimGraphInAir InAir;

	/** Anim Graph - Aiming Values */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Aiming Values", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimGraphAimingValues AimingValues;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Aiming Values")
	FVector2D SmoothedAimingAngle = FVector2D::ZeroVector;

	/** Anim Graph - Ragdoll */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Ragdoll")
	float FlailRate = 0.0f;

	/** Anim Graph - Layer Blending */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Layer Blending", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimGraphLayerBlending LayerBlendingValues;

	/** Anim Graph - Foot IK */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Read Only Data|Anim Graph - Foot IK", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimGraphFootIK FootIKValues;

	/** Turn In Place */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Turn In Place", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimTurnInPlace TurnInPlaceValues;

	/** Rotate In Place */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Rotate In Place", Meta = (
		    ShowOnlyInnerProperties))
	FALSAnimRotateInPlace RotateInPlace;

	/** Configuration */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Main Configuration", Meta = (
		ShowOnlyInnerProperties))
	FALSAnimConfiguration Config;

	/** Blend Curves */

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> DiagonalScaleAmountCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_N_Walk = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_N_Run = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> StrideBlend_C_Walk = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> LandPredictionCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveFloat> LeanInAirCurve = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveVector> YawOffset_FB = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Blend Curves")
	TObjectPtr<UCurveVector> YawOffset_LR = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_R = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Dynamic Transition")
	TObjectPtr<UAnimSequenceBase> TransitionAnim_L = nullptr;

	/** IK Bone Names */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Anim Graph - Foot IK")
	FName IkFootL_BoneName = FName(TEXT("ik_foot_l"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Configuration|Anim Graph - Foot IK")
	FName IkFootR_BoneName = FName(TEXT("ik_foot_r"));



private:
	FTimerHandle OnPivotTimer;

	FTimerHandle PlayDynamicTransitionTimer;

	FTimerHandle OnJumpedTimer;

	bool bCanPlayDynamicTransition = true;

	UPROPERTY()
	TObjectPtr<UALSDebugComponent> ALSDebugComponent = nullptr;


};
