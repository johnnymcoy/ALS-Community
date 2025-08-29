// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ALSBPLib.generated.h"

/**
 * 
 */
UCLASS()
class ALSV4_CPP_API UALSBPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


	UFUNCTION(BlueprintCallable, Category = "ALS|BP Lib")
	static UActorComponent* GetALSDebugComponent(APlayerController* Controller);
	//~		~//
};
