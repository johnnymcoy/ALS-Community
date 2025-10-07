// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#include "Components/ALSDebugComponent.h"

#include "CollisionShape.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Blueprint/UserWidget.h"
#include "BPLib/BaseHelpersBPLib.h"
#include "Interfaces/ALSCharacterInterface.h"
#include "Library/ALSCharacterStructLibrary.h"
#include "Interfaces/ALSCameraBehaviorInterface.h"
#include "Interfaces/ALSCameraInterface.h"
#include "Interfaces/ALSDebugHUD.h"
#include "Interfaces/CustomCharacter.h"

bool UALSDebugComponent::bDebugView = false;
bool UALSDebugComponent::bShowTraces = false;
bool UALSDebugComponent::bShowDebugShapes = false;
bool UALSDebugComponent::bShowLayerColors = false;

DECLARE_CYCLE_STAT(TEXT("ALS Debug Component (All Functions)"), STAT_ALS_Debug_Component, STATGROUP_ALS);
DECLARE_CYCLE_STAT(TEXT("ALS Debug Component Tick"), STAT_ALS_Debug_Component_Tick, STATGROUP_ALS);


UALSDebugComponent::UALSDebugComponent()
{
	ALSHUDClass = UBaseHelpersBPLib::GetDefaultWidgetBPClass("/ALSV4_CPP/AdvancedLocomotionV4/Blueprints/UI/ALS_HUD");
	OverlayStateSwitcherClass = UBaseHelpersBPLib::GetDefaultWidgetBPClass("/ALSV4_CPP/AdvancedLocomotionV4/Blueprints/UI/OverlayStateSwitcher");
	bAutoActivate = true;
#if UE_BUILD_SHIPPING
	PrimaryComponentTick.bCanEverTick = false;
#else
	PrimaryComponentTick.bCanEverTick = true;
#endif
}

void UALSDebugComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSDebugComponent::TickComponent);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Debug_Component);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Debug_Component_Tick);
	if(IsActive() == false){return;}
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if !UE_BUILD_SHIPPING
	if (!OwnerCharacter || !OwnerALSCharacter)
	{
		return;
	}

	if (bNeedsColorReset)
	{
		bNeedsColorReset = false;
		SetResetColors();
	}

	if (bShowLayerColors)
	{
		UpdateColoringSystem();
	}
	else
	{
		bNeedsColorReset = true;
	}

	if (bShowDebugShapes)
	{
		DrawDebugSpheres();

		APlayerController* Controller = Cast<APlayerController>(OwnerCharacter->GetController());
		if(Controller)
		{
			// AALSPlayerCameraManager* CamManager = Cast<AALSPlayerCameraManager>(Controller->PlayerCameraManager);
			IALSCameraInterface* CamManager = Cast<IALSCameraInterface>(Controller->PlayerCameraManager);
			if(CamManager)
			{
				CamManager->RequestDrawDebugTargets(OwnerALSCharacter->GetThirdPersonPivotTarget().GetLocation());
			}
		}
	}
#endif
}

void UALSDebugComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSDebugComponent::OnComponentDestroyed);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Debug_Component);

	Super::OnComponentDestroyed(bDestroyingHierarchy);

	// Keep static values false on destroy
	bDebugView = false;
	bShowTraces = false;
	bShowDebugShapes = false;
	bShowLayerColors = false;
}

void UALSDebugComponent::FocusedDebugCharacterCycle(bool bValue)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSDebugComponent::FocusedDebugCharacterCycle);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Debug_Component);

	// Refresh list, so we can also debug runtime spawned characters & remove despawned characters back
	DetectDebuggableCharactersInWorld();

	if (FocusedDebugCharacterIndex == INDEX_NONE)
	{
		// Return here as no AALSBaseCharacter where found during call of BeginPlay.
		// Moreover, for safety set also no focused debug character.
		DebugFocusCharacter = nullptr;
		return;
	}

	if (bValue)
	{
		FocusedDebugCharacterIndex++;
		if (FocusedDebugCharacterIndex >= AvailableDebugCharacters.Num())
		{
			FocusedDebugCharacterIndex = 0;
		}
	}
	else
	{
		FocusedDebugCharacterIndex--;
		if (FocusedDebugCharacterIndex < 0)
		{
			FocusedDebugCharacterIndex = AvailableDebugCharacters.Num() - 1;
		}
	}
	DebugFocusCharacter = AvailableDebugCharacters[FocusedDebugCharacterIndex];
}

void UALSDebugComponent::BeginPlay()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSDebugComponent::BeginPlay);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Debug_Component);

	Super::BeginPlay();
	OwnerCharacter = Cast<APawn>(GetOwner());
	OwnerALSCharacter = Cast<IALSCharacterInterface>(GetOwner());
	DebugFocusCharacter = OwnerCharacter;
	if(OwnerCharacter)
	{
		SetDynamicMaterials();
		SetResetColors();
	}
}

void UALSDebugComponent::InitializePlayerController(APlayerController* Controller)
{
	if(Controller == nullptr || Controller->IsLocalController() == false || ALSHUDClass == nullptr){return;}
	ALSHUD = nullptr;
	if(bCreateDebugHUD)
	{
		ALSHUD = CreateWidget(Controller, ALSHUDClass);
		if(ALSHUD != nullptr)
		{
			ALSHUD->AddToPlayerScreen();
		}
	}
	OnPlayerControllerInitialized(Controller);
}

void UALSDebugComponent::DetectDebuggableCharactersInWorld()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSDebugComponent::DetectDebuggableCharactersInWorld);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Debug_Component);

	// Get all ALSBaseCharacter's, which are currently present to show them later in the ALS HUD for debugging purposes.
	TArray<AActor*> AlsBaseCharacters;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UALSCharacterInterface::StaticClass(), AlsBaseCharacters);
	// UGameplayStatics::GetAllActorsOfClass(GetWorld(), AALSBaseCharacter::StaticClass(), AlsBaseCharacters);
	AvailableDebugCharacters.Empty();
	if (AlsBaseCharacters.Num() > 0)
	{
		AvailableDebugCharacters.Reserve(AlsBaseCharacters.Num());
		for(AActor* Character : AlsBaseCharacters)
		{
			if(Character == nullptr){continue;}
			if(IALSCharacterInterface* AlsBaseCharacter = Cast<IALSCharacterInterface>(Character))
			// if(AALSBaseCharacter* AlsBaseCharacter = Cast<AALSBaseCharacter>(Character))
			{
				AvailableDebugCharacters.Add(Character);
			}
		}
		FocusedDebugCharacterIndex = AvailableDebugCharacters.Find(DebugFocusCharacter);
		if (FocusedDebugCharacterIndex == INDEX_NONE && AvailableDebugCharacters.Num() > 0)
		{ // seems to be that this component was not attached to and AALSBaseCharacter,
			// therefore the index will be set to the first element in the array.
			FocusedDebugCharacterIndex = 0;
		}
	}
}

IALSDebugHUD* UALSDebugComponent::GetDebugHUD() const
{
	if(ALSHUD == nullptr){return nullptr;}
	return Cast<IALSDebugHUD>(ALSHUD);
}

void UALSDebugComponent::ToggleGlobalTimeDilationLocal(float TimeDilation)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSDebugComponent::ToggleGlobalTimeDilationLocal);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Debug_Component);

	if (UKismetSystemLibrary::IsStandalone(this))
	{
		UGameplayStatics::SetGlobalTimeDilation(this, TimeDilation);
	}
}

void UALSDebugComponent::ToggleSlomo()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSDebugComponent::ToggleSlomo);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Debug_Component);

	bSlomo = !bSlomo;
	ToggleGlobalTimeDilationLocal(bSlomo ? 0.15f : 1.f);
}

void UALSDebugComponent::ToggleDebugView()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSDebugComponent::ToggleDebugView);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Debug_Component);
	bDebugView = !bDebugView;
	if(CameraBehavior)
	{
		CameraBehavior->SetDebugView(bDebugView);
	}
}

void UALSDebugComponent::OpenOverlayMenu(bool bValue)
{
	bOverlayMenuOpen = bValue;
	if(OverlayStateSwitcherClass == nullptr){UE_LOG(LogTemp, Warning, TEXT("ALSDebug:OpenOverlayMenu OverlayStateSwitcherClass null"));return;}
	if(bValue)
	{
		ToggleGlobalTimeDilationLocal(0.35f);
		OverlayStateSwitcher = CreateWidget(GetWorld(), OverlayStateSwitcherClass);
		if(OverlayStateSwitcher != nullptr)
		{
			OverlayStateSwitcher->AddToPlayerScreen();
		}
	}
	else
	{
		ToggleGlobalTimeDilationLocal(1.0f);
		if(OverlayStateSwitcher != nullptr)
		{
			OverlayStateSwitcher->RemoveFromParent();
			OverlayStateSwitcher = nullptr;
		}
	}
	OnOpenOverlayMenu(bValue);
}

void UALSDebugComponent::OverlayMenuCycle(bool bValue)
{
	OnOverlayMenuCycle(bValue);
}

void UALSDebugComponent::ToggleDebugMesh()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSDebugComponent::ToggleDebugMesh);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Debug_Component);
	//@TODO Toggle Between ALS mesh and Other Skeletal Meshes
	if(OwnerCharacter == nullptr){return;}
	const ICharacterBaseInterface* BaseCharacter = Cast<ICharacterBaseInterface>(OwnerCharacter);
	if(BaseCharacter == nullptr){return;}
	BaseCharacter->GetCharacterMesh()->SetVisibility(bDebugMeshVisible);
	bDebugMeshVisible = !bDebugMeshVisible;
}


void UALSDebugComponent::SetCameraBehavior(UObject* CameraBehaviorRef)
{
	if(CameraBehaviorRef == nullptr){return;}
	CameraBehavior = Cast<IALSCameraBehaviorInterface>(CameraBehaviorRef);
	if(CameraBehavior == nullptr){UE_LOG(LogTemp, Error, TEXT("SetCameraBehavior Camera Behavior failed"));}
}

/** Util for drawing result of single line trace  */
void UALSDebugComponent::DrawDebugLineTraceSingle(const UWorld* World,
	                                                const FVector& Start,
	                                                const FVector& End,
	                                                EDrawDebugTrace::Type
	                                                DrawDebugType,
	                                                bool bHit,
	                                                const FHitResult& OutHit,
	                                                FLinearColor TraceColor,
	                                                FLinearColor TraceHitColor,
	                                                float DrawTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSDebugComponent::DrawDebugLineTraceSingle);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Debug_Component);

	if (DrawDebugType != EDrawDebugTrace::None)
	{
		bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
		float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawTime : 0.f;

		if (bHit && OutHit.bBlockingHit)
		{
			// Red up to the blocking hit, green thereafter
			DrawDebugLine(World, Start, OutHit.ImpactPoint, TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugLine(World, OutHit.ImpactPoint, End, TraceHitColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 16.0f, TraceColor.ToFColor(true), bPersistent, LifeTime);
		}
		else
		{
			// no hit means all red
			DrawDebugLine(World, Start, End, TraceColor.ToFColor(true), bPersistent, LifeTime);
		}
	}
}

void UALSDebugComponent::DrawDebugLineTraceSingle_Local(const UWorld* World, const FVector& Start, const FVector& End,
	EDrawDebugTrace::Type DrawDebugType, bool bHit, const FHitResult& OutHit, FLinearColor TraceColor,
	FLinearColor TraceHitColor, float DrawTime)
{
	DrawDebugLineTraceSingle(World, Start, End, DrawDebugType, bHit, OutHit, TraceColor, TraceHitColor, DrawTime);
}

void UALSDebugComponent::DrawDebugCapsuleTraceSingle(const UWorld* World,
                                                     const FVector& Start,
                                                     const FVector& End,
                                                     const FCollisionShape& CollisionShape,
                                                     EDrawDebugTrace::Type DrawDebugType,
                                                     bool bHit,
                                                     const FHitResult& OutHit,
                                                     FLinearColor TraceColor,
                                                     FLinearColor TraceHitColor,
                                                     float DrawTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSDebugComponent::DrawDebugCapsuleTraceSingle);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Debug_Component);

	if (DrawDebugType != EDrawDebugTrace::None)
	{
		bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
		float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawTime : 0.f;

		if (bHit && OutHit.bBlockingHit)
		{
			// Red up to the blocking hit, green thereafter
			DrawDebugCapsule(World, Start, CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(), FQuat::Identity, TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugCapsule(World, OutHit.Location, CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(), FQuat::Identity, TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugLine(World, Start, OutHit.Location, TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 16.0f, TraceColor.ToFColor(true), bPersistent, LifeTime);

			DrawDebugCapsule(World, End, CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(), FQuat::Identity, TraceHitColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugLine(World, OutHit.Location, End, TraceHitColor.ToFColor(true), bPersistent, LifeTime);
		}
		else
		{
			// no hit means all red
			DrawDebugCapsule(World, Start, CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(), FQuat::Identity, TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugCapsule(World, End, CollisionShape.GetCapsuleHalfHeight(), CollisionShape.GetCapsuleRadius(), FQuat::Identity, TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugLine(World, Start, End, TraceColor.ToFColor(true), bPersistent, LifeTime);
		}
	}
}

static void DrawDebugSweptSphere(const UWorld* InWorld,
	                        FVector const& Start,
	                        FVector const& End,
	                        float Radius,
	                        FColor const& Color,
	                        bool bPersistentLines = false,
	                        float LifeTime = -1.f,
	                        uint8 DepthPriority = 0)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSDebugComponent::DrawDebugSweptSphere);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Debug_Component);

	FVector const TraceVec = End - Start;
	float const Dist = TraceVec.Size();

	FVector const Center = Start + TraceVec * 0.5f;
	float const HalfHeight = (Dist * 0.5f) + Radius;

	FQuat const CapsuleRot = FRotationMatrix::MakeFromZ(TraceVec).ToQuat();
	DrawDebugCapsule(InWorld, Center, HalfHeight, Radius, CapsuleRot, Color, bPersistentLines, LifeTime, DepthPriority);
}

void UALSDebugComponent::DrawDebugSphereTraceSingle(const UWorld* World,
	                                                  const FVector& Start,
	                                                  const FVector& End,
	                                                  const FCollisionShape& CollisionShape,
	                                                  EDrawDebugTrace::Type DrawDebugType,
	                                                  bool bHit,
	                                                  const FHitResult& OutHit,
	                                                  FLinearColor TraceColor,
	                                                  FLinearColor TraceHitColor,
	                                                  float DrawTime)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UALSDebugComponent::DrawDebugSphereTraceSingle);
	SCOPE_CYCLE_COUNTER(STAT_ALS_Debug_Component);

	if (DrawDebugType != EDrawDebugTrace::None)
	{
		bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
		float LifeTime = (DrawDebugType == EDrawDebugTrace::ForDuration) ? DrawTime : 0.f;

		if (bHit && OutHit.bBlockingHit)
		{
			// Red up to the blocking hit, green thereafter
			DrawDebugSweptSphere(World, Start, OutHit.Location, CollisionShape.GetSphereRadius(), TraceColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugSweptSphere(World, OutHit.Location, End, CollisionShape.GetSphereRadius(), TraceHitColor.ToFColor(true), bPersistent, LifeTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 16.0f, TraceColor.ToFColor(true), bPersistent, LifeTime);
		}
		else
		{
			// no hit means all red
			DrawDebugSweptSphere(World, Start, End, CollisionShape.GetSphereRadius(), TraceColor.ToFColor(true), bPersistent, LifeTime);
		}
	}
}

void UALSDebugComponent::DrawDebugSphereTraceSingle_Local(const UWorld* World, const FVector& Start, const FVector& End,
	const FCollisionShape& CollisionShape, EDrawDebugTrace::Type DrawDebugType, bool bHit, const FHitResult& OutHit,
	FLinearColor TraceColor, FLinearColor TraceHitColor, float DrawTime)
{
	DrawDebugSphereTraceSingle(World, Start, End, CollisionShape, DrawDebugType, bHit, OutHit,
	                                                     TraceColor, TraceHitColor, DrawTime);
}
