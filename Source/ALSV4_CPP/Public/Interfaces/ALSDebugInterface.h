// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ALSDebugInterface.generated.h"

namespace EDrawDebugTrace
{
	enum Type : int;
}

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UALSDebugInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @class IALSDebugInterface
 * @brief Interface for managing and debugging ALS (Advanced Locomotion System) features.
 *
 * Defines methods for toggling debug options, managing camera behavior, and debugging trace shapes.
 */
class IALSDebugInterface
{
	GENERATED_BODY()

public:
	virtual void InitializePlayerController(APlayerController* Controller) = 0;
	virtual void ToggleHUD() = 0;
	virtual void ToggleDebugView() = 0;
	virtual void ToggleTraces() = 0;
	virtual void ToggleDebugShapes() = 0;
	virtual void ToggleLayerColors() = 0;
	virtual void ToggleCharacterInfo() = 0;
	virtual void ToggleSlomo() = 0;
	virtual void FocusedDebugCharacterCycle(bool bValue) = 0;
	virtual void ToggleDebugMesh() = 0;
	virtual void OpenOverlayMenu(bool bValue) = 0;
	virtual void OverlayMenuCycle(bool bValue) = 0;

	virtual void SetCameraBehavior(UObject* CameraBehaviorRef) = 0;

	UFUNCTION(BlueprintCallable, Category = "ALS|Debug")
	virtual bool GetShowLayerColors() = 0;

	
	UFUNCTION(BlueprintCallable, Category = "ALS|Debug")
	virtual bool GetShowTraces() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Debug")
	virtual bool GetDebugView() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Debug")
	virtual bool GetShowDebugShapes() const = 0;

	UFUNCTION(BlueprintCallable, Category = "ALS|Debug")
	virtual bool GetShowHUD() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Debug")
	virtual bool GetShowCharacterInfo() const = 0;
	UFUNCTION(BlueprintCallable, Category = "ALS|Debug")
	virtual bool GetSlowmo() const = 0;


	virtual void DrawDebugSphereTraceSingle_Local(const UWorld* World,
									   const FVector& Start,
									   const FVector& End,
									   const FCollisionShape& CollisionShape,
									   EDrawDebugTrace::Type DrawDebugType,
									   bool bHit,
									   const FHitResult& OutHit,
									   FLinearColor TraceColor,
									   FLinearColor TraceHitColor,
									   float DrawTime) = 0;
	virtual void DrawDebugLineTraceSingle_Local(const UWorld* World,
										 const FVector& Start,
										 const FVector& End,
										 EDrawDebugTrace::Type DrawDebugType,
										 bool bHit,
										 const FHitResult& OutHit,
										 FLinearColor TraceColor,
										 FLinearColor TraceHitColor,
										 float DrawTime) = 0;

};
