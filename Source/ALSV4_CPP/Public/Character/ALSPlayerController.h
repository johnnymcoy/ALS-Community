// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Character/PlayerControllerBase.h"
#include "Characters/InteractionPlayerController.h"
#include "ALSPlayerController.generated.h"

class IALSDebugInterface;
class IALSCharacterInput;
class AALSBaseCharacter;
class UInputMappingContext;

/**
 * Player controller class
 */
UCLASS(Blueprintable, BlueprintType)
class ALSV4_CPP_API AALSPlayerController : public AInteractionPlayerController
{
	GENERATED_BODY()

public:
	virtual void OnPossess(APawn* NewPawn) override;

	virtual void OnRep_Pawn() override;

	virtual void SetupInputComponent() override;

	virtual void BindActions(UInputMappingContext* Context) override;

protected:
	virtual void SetupInputs() override;

	void SetupCamera();

	//@ TODO Remove These 
	// virtual void ForwardMovementAction(const FInputActionValue& Value) override;
	// virtual void RightMovementAction(const FInputActionValue& Value) override;
	// virtual void CameraUpAction(const FInputActionValue& Value) override;
	// virtual void CameraRightAction(const FInputActionValue& Value) override;
	// virtual void JumpAction(const FInputActionValue& Value) override;
	// virtual void AimAction(const FInputActionValue& Value) override;

	UFUNCTION()
	void SprintAction(const FInputActionValue& Value);
	

	UFUNCTION()
	void CameraTapAction(const FInputActionValue& Value);

	UFUNCTION()
	void CameraHeldAction(const FInputActionValue& Value);

	UFUNCTION()
	void StanceAction(const FInputActionValue& Value);

	UFUNCTION()
	void WalkAction(const FInputActionValue& Value);

	UFUNCTION()
	void RagdollAction(const FInputActionValue& Value);

	UFUNCTION()
	void VelocityDirectionAction(const FInputActionValue& Value);

	UFUNCTION()
	void LookingDirectionAction(const FInputActionValue& Value);

	// Debug actions
	UFUNCTION()
	void DebugToggleHudAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugToggleDebugViewAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugToggleTracesAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugToggleShapesAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugToggleLayerColorsAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugToggleCharacterInfoAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugToggleSlomoAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugFocusedCharacterCycleAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugToggleMeshAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugOpenOverlayMenuAction(const FInputActionValue& Value);

	UFUNCTION()
	void DebugOverlayMenuCycleAction(const FInputActionValue& Value);

public:
	/** Main character reference */
	// UPROPERTY(BlueprintReadOnly, Category = "ALS")
	// TObjectPtr<AALSBaseCharacter> PossessedCharacter = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Controller|Input|ALS")
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Controller|Input|ALS")
	TObjectPtr<UInputMappingContext> DebugInputMappingContext = nullptr;

private:
	IALSCharacterInput* GetALSCharacterInput();
	IALSDebugInterface* GetALSDebugInterface();

	IALSCharacterInput* CharacterALSInterface = nullptr;
	IALSDebugInterface* DebugALSInterface = nullptr;


};
