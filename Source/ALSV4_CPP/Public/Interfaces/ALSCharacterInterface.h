// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// #include "ALSCharacterMovementInterface.h"
#include "ALSGravityMovementInterface.h"
// #include "Library/ALSExtraData.h"
#include "UObject/Interface.h"
#include "ALSCharacterInterface.generated.h"

class IALSCameraBehaviorInterface;
enum class EALSOverlayState : uint8;
enum class EALSMovementState : uint8;
enum class EALSRotationMode : uint8;
enum class EALSMovementAction : uint8;
enum class EALSGait : uint8;
enum class EALSStance : uint8;
enum class EALSViewMode : uint8;

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

	//~~ For Animations			~//
	virtual FVector GetLeftHandGoal() const = 0;
	virtual FVector GetRightHandGoal() const = 0;
	virtual EALSOverlayState GetCurrentOverlayState() const = 0;
	virtual bool GetIsMantling() const = 0;
	//~~ For Animations			~//

	virtual IALSGravityMovementInterface* GetALSGravityMovementInterface() const = 0;

	//- For Camera Manager		~//
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual FTransform GetThirdPersonPivotTarget() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual FVector GetFirstPersonCameraTarget() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual FVector GetAimDownSightCameraTarget() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual float GetAimDownSightFOV() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Camera System")
	virtual void GetCameraParameters(float& TPFOVOut, float& FPFOVOut, bool& bRightShoulderOut) const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Camera System")
	virtual ECollisionChannel GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius) = 0;
	// UFUNCTION(BlueprintCallable, Category = "ALS|Camera System")
	// virtual FVector GetFirstPersonCameraTarget();

	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual EALSMovementState GetMovementState() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual EALSStance GetStance() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual EALSMovementAction GetMovementAction() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual bool IsRightShoulder() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual EALSGait GetGait() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual EALSRotationMode GetRotationMode() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual bool IsAimingDownSights() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual EALSViewMode GetViewMode() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual EALSOverlayState GetOverlayState() const = 0;

	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual void GetCapsuleSize(float& ScaledHalfHeight, float& Radius) const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual FVector GetSocketLocation(FName SocketName) const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual FRotator GetTargetRotation() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual FRotator GetLastVelocityRotation() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual FRotator GetLastMovementInputRotation() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual FVector GetCharacterAcceleration(float& MaxAcceleration) const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual float GetMaxWalkSpeed() const = 0;

	UFUNCTION(BlueprintCallable, Category = "ALS|Utility")
	virtual float GetAnimCurveValue(FName CurveName) const = 0;

	UFUNCTION(BlueprintCallable, Category = "ALS|Character States")
	virtual void SetOverlayState(EALSOverlayState NewState, bool bForce = false) = 0;

	virtual void SetCameraBehavior(UObject* CameraBehaviorRef) = 0;
	// virtual void SetCameraBehavior(UALSPlayerCameraBehavior* CamBeh);
	// virtual void GravityDirectionChanged(EGravityDirectionMode OldGravityDirectionMode, EGravityDirectionMode CurrentGravityDirectionMode) = 0;


};
