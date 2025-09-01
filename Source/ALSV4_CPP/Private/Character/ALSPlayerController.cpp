// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community


#include "Character/ALSPlayerController.h"
// #include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/ALSCameraInterface.h"
#include "Interfaces/ALSCharacterInput.h"
#include "Interfaces/ALSCharacterInterface.h"
#include "Interfaces/ALSDebugInterface.h"
#include "Library/ALSCharacterEnumLibrary.h"

AALSPlayerController::AALSPlayerController()
{
	ConstructorHelpers::FObjectFinder<UInputMappingContext> const DefaultMappingBP(TEXT("/ALSV4_CPP/AdvancedLocomotionV4/Blueprints/Input/IMC_Default"));
	if (!ensure(DefaultMappingBP.Object != nullptr)) return;
	DefaultInputMappingContext = DefaultMappingBP.Object;
	ConstructorHelpers::FObjectFinder<UInputMappingContext> const DebugMappingBP(TEXT("/ALSV4_CPP/AdvancedLocomotionV4/Blueprints/Input/IMC_Debug"));
	if (!ensure(DebugMappingBP.Object != nullptr)) return;
	DebugInputMappingContext = DebugMappingBP.Object;

}

void AALSPlayerController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);
	if (!IsRunningDedicatedServer())
	{
		// Servers want to setup camera only in listen servers.
		SetupCamera();
	}
	GetALSCharacterInput();
	GetALSCharacterInterface();
	GetALSDebugInterface();
	if(GetALSDebugInterface() != nullptr)
	{
		GetALSDebugInterface()->InitializePlayerController(this);
	}
}

void AALSPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
	ALSCharacterInput = nullptr;
	DebugALSInterface = nullptr;
	ALSCharacterInterface = nullptr;
}

void AALSPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	GetALSCharacterInput();
	GetALSCharacterInterface();
	GetALSDebugInterface();
	SetupCamera();
	// SetupInputs();

	if(GetALSDebugInterface() != nullptr)
	{
		GetALSDebugInterface()->InitializePlayerController(this);
	}
}

void AALSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	//! Overridden in Custom Player Controller 
	// UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	// if(EnhancedInputComponent == nullptr)
	// {
	// 	UE_LOG(LogTemp, Fatal, TEXT("ALS Community requires Enhanced Input System to be activated in project settings to function properly"));
	// 	return;
	// }
	// EnhancedInputComponent->ClearActionEventBindings();
	// EnhancedInputComponent->ClearActionValueBindings();
	// EnhancedInputComponent->ClearDebugKeyBindings();
	// BindActions(DefaultInputMappingContext);
	// BindActions(DebugInputMappingContext);
}

void AALSPlayerController::BindActions(UInputMappingContext* Context)
{
	Super::BindActions(Context);
	// if (Context)
	// {
	// 	const TArray<FEnhancedActionKeyMapping>& Mappings = Context->GetMappings();
	// 	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent);
	// 	if (EnhancedInputComponent)
	// 	{
	// 		// There may be more than one keymapping assigned to one action. So, first filter duplicate action entries to prevent multiple delegate bindings
	// 		TSet<const UInputAction*> UniqueActions;
	// 		for (const FEnhancedActionKeyMapping& Keymapping : Mappings)
	// 		{
	// 			UniqueActions.Add(Keymapping.Action);
	// 		}
	// 		for (const UInputAction* UniqueAction : UniqueActions)
	// 		{
	// 			EnhancedInputComponent->BindAction(UniqueAction, ETriggerEvent::Triggered, Cast<UObject>(this), UniqueAction->GetFName());
	// 		}
	// 	}
	// }
}

void AALSPlayerController::SetupInputs()
{
	Super::SetupInputs();
	// if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	// {
	// 	FModifyContextOptions Options;
	// 	Options.bForceImmediately = 1;
	// 	Subsystem->AddMappingContext(DefaultInputMappingContext, 1, Options);
	// 	// Subsystem->AddMappingContext(DebugInputMappingContext, 2, Options);
	// 	BindActions(DefaultInputMappingContext);
	// 	// BindActions(DebugInputMappingContext);
	// }
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		FModifyContextOptions Options;
		Options.bForceImmediately = 1;
		Subsystem->AddMappingContext(DefaultInputMappingContext, 3, Options);
		if(GetALSDebugInterface() != nullptr)
		{
			// Do only if we have debug component
			Subsystem->AddMappingContext(DebugInputMappingContext, 0, Options);
			BindActions(DebugInputMappingContext);
		}
		BindActions(DefaultInputMappingContext);
	}
	// } 
}

void AALSPlayerController::SetupCamera()
{
	// Call "OnPossess" in Player Camera Manager when possessing a pawn
	IALSCameraInterface* CastedMgr = Cast<IALSCameraInterface>(PlayerCameraManager);
	if(CastedMgr != nullptr && GetPawn() != nullptr)
	{
		CastedMgr->Possess(GetPawn());
	}
	// AALSPlayerCameraManager* CastedMgr = Cast<AALSPlayerCameraManager>(PlayerCameraManager);
	// if(PossessedCharacter && CastedMgr)
	// {
	// 	CastedMgr->OnPossess(PossessedCharacter);
	// }
}

void AALSPlayerController::SprintAction(const FInputActionValue& Value)
{
	if(GetALSCharacterInput())
	{
		GetALSCharacterInput()->SprintAction(Value.Get<bool>());
	}
}

void AALSPlayerController::CameraTapAction(const FInputActionValue& Value)
{
	if(GetALSCharacterInput())
	{
		GetALSCharacterInput()->CameraTapAction();
	}
}

void AALSPlayerController::CameraHeldAction(const FInputActionValue& Value)
{
	if(GetALSCharacterInput())
	{
		GetALSCharacterInput()->CameraHeldAction();
	}
}

void AALSPlayerController::StanceAction(const FInputActionValue& Value)
{
	if (GetALSCharacterInput() && Value.Get<bool>())
	{
		GetALSCharacterInput()->StanceAction();
	}
}

void AALSPlayerController::WalkAction(const FInputActionValue& Value)
{
	if (GetALSCharacterInput() && Value.Get<bool>())
	{
		GetALSCharacterInput()->WalkAction();
	}
}

void AALSPlayerController::RagdollAction(const FInputActionValue& Value)
{
	if (GetALSCharacterInput() && Value.Get<bool>())
	{
		GetALSCharacterInput()->RagdollAction();
	}
}

void AALSPlayerController::VelocityDirectionAction(const FInputActionValue& Value)
{
	if (GetALSCharacterInput() && Value.Get<bool>())
	{
		GetALSCharacterInput()->VelocityDirectionAction();
	}
}

void AALSPlayerController::LookingDirectionAction(const FInputActionValue& Value)
{
	if (GetALSCharacterInput() && Value.Get<bool>())
	{
		GetALSCharacterInput()->LookingDirectionAction();
	}
}

void AALSPlayerController::DebugToggleHudAction(const FInputActionValue& Value)
{
	if (GetALSDebugInterface() && Value.Get<bool>())
	{
		GetALSDebugInterface()->ToggleHUD();
	}
}

void AALSPlayerController::DebugToggleDebugViewAction(const FInputActionValue& Value)
{
	if (GetALSDebugInterface() && Value.Get<bool>())
	{
		GetALSDebugInterface()->ToggleDebugView();
	}
}

void AALSPlayerController::DebugToggleTracesAction(const FInputActionValue& Value)
{
	if (GetALSDebugInterface() && Value.Get<bool>())
	{
		GetALSDebugInterface()->ToggleTraces();
	}
}

void AALSPlayerController::DebugToggleShapesAction(const FInputActionValue& Value)
{
	if (GetALSDebugInterface() && Value.Get<bool>())
	{
		GetALSDebugInterface()->ToggleDebugShapes();
	}
}

void AALSPlayerController::DebugToggleLayerColorsAction(const FInputActionValue& Value)
{
	if (GetALSDebugInterface() && Value.Get<bool>())
	{
		GetALSDebugInterface()->ToggleLayerColors();
	}
}

void AALSPlayerController::DebugToggleCharacterInfoAction(const FInputActionValue& Value)
{
	if (GetALSDebugInterface() && Value.Get<bool>())
	{
		GetALSDebugInterface()->ToggleCharacterInfo();
	}
}

void AALSPlayerController::DebugToggleSlomoAction(const FInputActionValue& Value)
{
	if (GetALSDebugInterface() && Value.Get<bool>())
	{
		GetALSDebugInterface()->ToggleSlomo();
	}
}

void AALSPlayerController::DebugFocusedCharacterCycleAction(const FInputActionValue& Value)
{
	if (GetALSDebugInterface())
	{
		GetALSDebugInterface()->FocusedDebugCharacterCycle(Value.GetMagnitude() > 0);
	}
}

void AALSPlayerController::DebugToggleMeshAction(const FInputActionValue& Value)
{
	if (GetALSDebugInterface() && Value.Get<bool>())
	{
		GetALSDebugInterface()->ToggleDebugMesh();
	}
}

void AALSPlayerController::DebugOpenOverlayMenuAction(const FInputActionValue& Value)
{
	if (GetALSDebugInterface())
	{
		GetALSDebugInterface()->OpenOverlayMenu(Value.Get<bool>());
	}
}

void AALSPlayerController::DebugOverlayMenuCycleAction(const FInputActionValue& Value)
{
	if (GetALSDebugInterface())
	{
		GetALSDebugInterface()->OverlayMenuCycle(Value.GetMagnitude() > 0);
	}
}

IALSCharacterInterface* AALSPlayerController::GetALSCharacterInterface()
{
	// SCOPE_CYCLE_COUNTER(STAT_ALS_CHARACTER);
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSPlayerController::GetALSCharacterInterface);
	if(ALSCharacterInterface == nullptr)
	{
		ALSCharacterInterface = Cast<IALSCharacterInterface>(GetPawn());
	}
	return ALSCharacterInterface;

}

IALSCharacterInterface* AALSPlayerController::GetALSCharacterInterface() const
{
	return ALSCharacterInterface;
}

bool AALSPlayerController::GetIsControllerSetupComplete() const
{
	return (Super::GetIsControllerSetupComplete()) && (ALSCharacterInput != nullptr) && (ALSCharacterInterface != nullptr);
}

float AALSPlayerController::CalculateCameraSensitivity(const float CameraInput) const
{
	float CameraSpeed = Super::CalculateCameraSensitivity(CameraInput);
	if(GetALSCharacterInterface() == nullptr)
	{
		switch(GetALSCharacterInterface()->GetOverlayState())
		{
		case EALSOverlayState::Default:
		case EALSOverlayState::Masculine:
		case EALSOverlayState::Feminine:
		case EALSOverlayState::Injured:
		case EALSOverlayState::HandsTied:
		case EALSOverlayState::Torch:
		case EALSOverlayState::Binoculars:
		case EALSOverlayState::Box:
		case EALSOverlayState::Barrel:
			break;
		case EALSOverlayState::Rifle:
			//@TODO Add these back in ?
			// CameraSpeed *= CameraSensitivityRifleMultiplier;
			break;
		case EALSOverlayState::PistolOneHanded:
			// CameraSpeed *= CameraSensitivityOneHandPistolMultiplier;
			break;
		case EALSOverlayState::PistolTwoHanded:
			// CameraSpeed *= CameraSensitivityTwoHandPistolMultiplier;
			break;
		case EALSOverlayState::Shotgun:
			// CameraSpeed *= CameraSensitivityShotgunMultiplier;
			break;
		case EALSOverlayState::Sniper:
			// CameraSpeed *= CameraSensitivitySniperMultiplier;
			break;
		case EALSOverlayState::Launcher:
			// CameraSpeed *= CameraSensitivityLauncherMultiplier;
			break;
		case EALSOverlayState::Sword:
		case EALSOverlayState::SwordShield:
		case EALSOverlayState::Knife:
		case EALSOverlayState::Chainsaw:
		case EALSOverlayState::Melee:
			// CameraSpeed *= CameraSensitivityMeleeMultiplier;
			break;
		case EALSOverlayState::Bow:
			// CameraSpeed *= CameraSensitivityBowMultiplier;
			break;
		}
	}
	return CameraSpeed;
}

IALSCharacterInput* AALSPlayerController::GetALSCharacterInput()
{
	// SCOPE_CYCLE_COUNTER(STAT_ALS_CHARACTER);
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSPlayerController::GetALSCharacterInput);
	if(ALSCharacterInput == nullptr)
	{
		ALSCharacterInput = Cast<IALSCharacterInput>(GetPawn());
	}
	return ALSCharacterInput;
}

IALSDebugInterface* AALSPlayerController::GetALSDebugInterface()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(AALSPlayerController::GetALSDebugInterface);
	if(DebugALSInterface == nullptr && GetPawn() != nullptr)
	{
		for(const auto& Component:GetPawn()->GetComponentsByInterface(UALSDebugInterface::StaticClass()))
		{
			if(Component == nullptr){continue;}
			IALSDebugInterface* DebugComponent = Cast<IALSDebugInterface>(Component);
			if(DebugComponent == nullptr){continue;}
			DebugALSInterface = DebugComponent;
			break;
		}
	}
	return DebugALSInterface;
}
