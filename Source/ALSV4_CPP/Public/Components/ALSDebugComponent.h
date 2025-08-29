// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#pragma once

#include "CoreMinimal.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Components/ActorComponent.h"
#include "Interfaces/ALSDebugInterface.h"
#include "ALSDebugComponent.generated.h"

class IALSCharacterInterface;
class AALSBaseCharacter;
class USkeletalMesh;

UCLASS(Blueprintable, BlueprintType)
class ALSV4_CPP_API UALSDebugComponent : public UActorComponent, public IALSDebugInterface
{
	GENERATED_BODY()

public:
	UALSDebugComponent();

	virtual void BeginPlay() override;

	virtual void InitializePlayerController(APlayerController* Controller) override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "ALS|Debug")
	void OnPlayerControllerInitialized(APlayerController* Controller);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;

	/** Implemented on BP to update layering colors */
	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|Debug")
	void UpdateColoringSystem();

	/** Implement on BP to draw debug spheres */
	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|Debug")
	void DrawDebugSpheres();

	/** Implemented on BP to set/reset layering colors */
	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|Debug")
	void SetResetColors();

	/** Implemented on BP to set dynamic color materials for debugging */
	UFUNCTION(BlueprintImplementableEvent, Category = "ALS|Debug")
	void SetDynamicMaterials();

	UFUNCTION(BlueprintCallable, Category = "ALS|Debug")
	void ToggleGlobalTimeDilationLocal(float TimeDilation);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "ALS|Debug", DisplayName="Open Overlay Menu")
	void OnOpenOverlayMenu(bool bValue);
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "ALS|Debug", DisplayName="Overlay Menu Cycle")
	void OnOverlayMenuCycle(bool bValue);


	virtual void ToggleSlomo() override;
	virtual void ToggleHUD() override { bShowHud = !bShowHud; }
	virtual void ToggleDebugView() override;
	virtual void OpenOverlayMenu(bool bValue) override;
	virtual void OverlayMenuCycle(bool bValue) override;
	virtual void ToggleDebugMesh() override;
	virtual void ToggleTraces() override { bShowTraces = !bShowTraces; }
	virtual void ToggleDebugShapes() override { bShowDebugShapes = !bShowDebugShapes; }
	virtual void ToggleLayerColors() override { bShowLayerColors = !bShowLayerColors; }
	virtual void ToggleCharacterInfo() override { bShowCharacterInfo = !bShowCharacterInfo; }
	virtual void FocusedDebugCharacterCycle(bool bValue) override;

	virtual bool GetDebugView() const override { return bDebugView; }
	virtual bool GetShowTraces() const override { return bShowTraces; }
	virtual bool GetShowDebugShapes() const override { return bShowDebugShapes; }
	virtual bool GetShowLayerColors() override { return bShowLayerColors; }
	virtual bool GetShowHUD() const override {return bShowHud;};
	virtual bool GetShowCharacterInfo()  const override {return bShowCharacterInfo;};
	virtual bool GetSlowmo() const override {return bSlomo;};

	virtual void SetCameraBehavior(UObject* CameraBehaviorRef) override;


	// utility functions to draw trace debug shapes,
	// which are derived from Engine/Private/KismetTraceUtils.h.
	// Sadly the functions are private, which was the reason
	// why there reimplemented here.
	static void DrawDebugLineTraceSingle(const UWorld* World,
	                                     const FVector& Start,
	                                     const FVector& End,
	                                     EDrawDebugTrace::Type DrawDebugType,
	                                     bool bHit,
	                                     const FHitResult& OutHit,
	                                     FLinearColor TraceColor,
	                                     FLinearColor TraceHitColor,
	                                     float DrawTime);
	virtual void DrawDebugLineTraceSingle_Local(const UWorld* World,
									 const FVector& Start,
									 const FVector& End,
									 EDrawDebugTrace::Type DrawDebugType,
									 bool bHit,
									 const FHitResult& OutHit,
									 FLinearColor TraceColor,
									 FLinearColor TraceHitColor,
									 float DrawTime) override;

	static void DrawDebugCapsuleTraceSingle(const UWorld* World,
	                                        const FVector& Start,
	                                        const FVector& End,
	                                        const FCollisionShape& CollisionShape,
	                                        EDrawDebugTrace::Type DrawDebugType,
	                                        bool bHit,
	                                        const FHitResult& OutHit,
	                                        FLinearColor TraceColor,
	                                        FLinearColor TraceHitColor,
	                                        float DrawTime);

	static void DrawDebugSphereTraceSingle(const UWorld* World,
	                                       const FVector& Start,
	                                       const FVector& End,
	                                       const FCollisionShape& CollisionShape,
	                                       EDrawDebugTrace::Type DrawDebugType,
	                                       bool bHit,
	                                       const FHitResult& OutHit,
	                                       FLinearColor TraceColor,
	                                       FLinearColor TraceHitColor,
	                                       float DrawTime);
	virtual void DrawDebugSphereTraceSingle_Local(const UWorld* World,
								   const FVector& Start,
								   const FVector& End,
								   const FCollisionShape& CollisionShape,
								   EDrawDebugTrace::Type DrawDebugType,
								   bool bHit,
								   const FHitResult& OutHit,
								   FLinearColor TraceColor,
								   FLinearColor TraceHitColor,
								   float DrawTime) override;


protected:
	void DetectDebuggableCharactersInWorld();

public:
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Debug")
	TObjectPtr<APawn> OwnerCharacter = nullptr;
	IALSCharacterInterface* OwnerALSCharacter = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Debug")
	bool bSlomo = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Debug")
	bool bShowHud = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Debug")
	bool bShowCharacterInfo = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ALS|Debug")
	TObjectPtr<USkeletalMesh> DebugSkeletalMesh = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Debug")
	TArray<TObjectPtr<AActor>> AvailableDebugCharacters;
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Debug")
	TObjectPtr<AActor> DebugFocusCharacter = nullptr;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Debug")
	bool bOverlayMenuOpen = false;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Debug|Widget")
	UUserWidget* ALSHUD = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "ALS|Debug|Widget")
	UUserWidget* OverlayStateSwitcher = nullptr;

private:
	static bool bDebugView;

	static bool bShowTraces;

	static bool bShowDebugShapes;

	static bool bShowLayerColors;

	bool bNeedsColorReset = false;

	bool bDebugMeshVisible = false;

	UPROPERTY(EditDefaultsOnly, Category = "ALS|Debug")
	TSubclassOf<UUserWidget> ALSHUDClass = nullptr;
	UPROPERTY(EditDefaultsOnly, Category = "ALS|Debug")
	TSubclassOf<UUserWidget> OverlayStateSwitcherClass = nullptr;

	UPROPERTY()
	TObjectPtr<USkeletalMesh> DefaultSkeletalMesh = nullptr;

	class IALSCameraBehaviorInterface* CameraBehavior = nullptr;

	/// Stores the index, which is used to select the next focused debug ALSBaseCharacter.
	/// If no characters where found during BeginPlay the value should be set to INDEX_NONE.
	int32 FocusedDebugCharacterIndex = INDEX_NONE;
};
