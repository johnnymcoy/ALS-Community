// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community


#include "Character/ALSCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "AI/ALSAIController.h"
#include "Kismet/GameplayStatics.h"

AALSCharacter::AALSCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
	// HeldObjectRoot = CreateDefaultSubobject<USceneComponent>(TEXT("HeldObjectRoot"));
	// HeldObjectRoot->SetupAttachment(GetMesh());

	HeldObjectRoot = CreateOptionalDefaultSubobject<USceneComponent>(TEXT("HeldObjectRoot"));
	// HeldObjectRoot->SetupAttachment(GetMesh());

	SkeletalMesh = CreateOptionalDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	// if(HeldObjectRoot != nullptr)
	// {
	// 	SkeletalMesh->SetupAttachment(HeldObjectRoot);
	// }

	// SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	// SkeletalMesh->SetupAttachment(HeldObjectRoot);
	StaticMesh = CreateOptionalDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	// if(HeldObjectRoot != nullptr)
	// {
	// 	StaticMesh->SetupAttachment(HeldObjectRoot);
	// }
	// StaticMesh->SetupAttachment(HeldObjectRoot);

	// StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	// StaticMesh->SetupAttachment(HeldObjectRoot);

	AIControllerClass = AALSAIController::StaticClass();
}

void AALSCharacter::ClearHeldObject()
{
	if(StaticMesh != nullptr)
	{
		StaticMesh->SetStaticMesh(nullptr);
	}
	if(SkeletalMesh != nullptr)
	{
		SkeletalMesh->SetSkeletalMesh(nullptr);
		SkeletalMesh->SetAnimInstanceClass(nullptr);
	}
}

void AALSCharacter::AttachToHand(UStaticMesh* NewStaticMesh, USkeletalMesh* NewSkeletalMesh, UClass* NewAnimClass,
                                 bool bLeftHand, FVector Offset)
{
	ClearHeldObject();

	if (IsValid(NewStaticMesh) && StaticMesh != nullptr)
	{
		StaticMesh->SetStaticMesh(NewStaticMesh);
	}
	else if (IsValid(NewSkeletalMesh) && SkeletalMesh != nullptr)
	{
		SkeletalMesh->SetSkeletalMesh(NewSkeletalMesh);
		if (IsValid(NewAnimClass))
		{
			SkeletalMesh->SetAnimInstanceClass(NewAnimClass);
		}
		
	}

	FName AttachBone;
	if (bLeftHand)
	{
		AttachBone = TEXT("VB LHS_ik_hand_gun");
	}
	else
	{
		AttachBone = TEXT("VB RHS_ik_hand_gun");
	}
	if(HeldObjectRoot != nullptr)
	{
		HeldObjectRoot->AttachToComponent(GetMesh(),
		                                  FAttachmentTransformRules::SnapToTargetNotIncludingScale, AttachBone);
		HeldObjectRoot->SetRelativeLocation(Offset);
	}
}

void AALSCharacter::RagdollStart()
{
	ClearHeldObject();
	Super::RagdollStart();
}

void AALSCharacter::RagdollEnd()
{
	Super::RagdollEnd();
	UpdateHeldObject();
}

ECollisionChannel AALSCharacter::GetThirdPersonTraceParams(FVector& TraceOrigin, float& TraceRadius)
{
	const FName CameraSocketName = bRightShoulder ? TEXT("TP_CameraTrace_R") : TEXT("TP_CameraTrace_L");
	TraceOrigin = GetMesh()->GetSocketLocation(CameraSocketName);
	TraceRadius = 15.0f;
	return ECC_Camera;
}

FTransform AALSCharacter::GetThirdPersonPivotTarget()
{
	return FTransform(GetActorRotation(),
	                  (GetMesh()->GetSocketLocation(TEXT("Head")) + GetMesh()->GetSocketLocation(TEXT("root"))) / 2.0f,
	                  FVector::OneVector);
}

FVector AALSCharacter::GetFirstPersonCameraTarget()
{
	return GetMesh()->GetSocketLocation(TEXT("FP_Camera"));
}

void AALSCharacter::OnOverlayStateChanged(EALSOverlayState PreviousState)
{
	Super::OnOverlayStateChanged(PreviousState);
	UpdateHeldObject();
}

void AALSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHeldObjectAnimations();
}

void AALSCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHeldObject();
}
