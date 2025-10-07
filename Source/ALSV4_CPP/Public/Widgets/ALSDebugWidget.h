// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/ALSDebugHUD.h"
#include "ALSDebugWidget.generated.h"

/**
 * 
 */
// UCLASS()
UCLASS(meta = (DisableNativeTick))
class ALSV4_CPP_API UALSDebugWidget : public UUserWidget, public IALSDebugHUD
{
	GENERATED_BODY()

	
public:
	explicit UALSDebugWidget(const FObjectInitializer& ObjectInitializer);
	

protected:
	virtual bool Initialize() override;
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget), Category="Debug|Widgets")
	class UOverlay* RootOverlay = nullptr;

private:


	//

	
};
