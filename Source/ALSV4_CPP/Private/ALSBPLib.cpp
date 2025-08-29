// Fill out your copyright notice in the Description page of Project Settings.


#include "ALSBPLib.h"

#include "Interfaces/ALSDebugInterface.h"

UActorComponent* UALSBPLib::GetALSDebugComponent(APlayerController* Controller)
{
	if(Controller == nullptr || Controller->GetPawn() == nullptr){return nullptr;}
	TArray<UActorComponent*> ActorComponents = Controller->GetPawn()->GetComponentsByInterface(UALSDebugInterface::StaticClass());
	for(const auto& Component : ActorComponents)
	{
		if(Component == nullptr){continue;}
		const IALSDebugInterface* DebugComponent = Cast<IALSDebugInterface>(Component);
		if(DebugComponent == nullptr){continue;}
		return Component;
	}
	return nullptr;
}
