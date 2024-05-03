// Copyright:       Copyright (C) 2022 Doğa Can Yanıkoğlu
// Source Code:     https://github.com/dyanikoglu/ALS-Community

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/ALSCharacterMovementInterface.h"
#include "Interfaces/ALSGravityMovementInterface.h"
#include "Library/ALSCharacterStructLibrary.h"

#include "ALSCharacterMovementComponent.generated.h"

enum class EGravityDirectionMode : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGravityDirectionChanged, EGravityDirectionMode, OldGravityDirectionMode, EGravityDirectionMode, CurrentGravityDirectionMode);

/**
 * Authoritative networked Character Movement
 */
UCLASS()
class ALSV4_CPP_API UALSCharacterMovementComponent : public UCharacterMovementComponent, public IALSCharacterMovementInterface, public IALSGravityMovementInterface
{
	GENERATED_UCLASS_BODY()

	class ALSV4_CPP_API FSavedMove_My : public FSavedMove_Character
	{
	public:

		typedef FSavedMove_Character Super;

		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel,
		                        class FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(class ACharacter* Character) override;

		// Walk Speed Update
		uint8 bSavedRequestMovementSettingsChange : 1;
		EALSGait SavedAllowedGait = EALSGait::Walking;
	};

	class ALSV4_CPP_API FNetworkPredictionData_Client_My : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_My(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual void OnMovementUpdated(float DeltaTime, const FVector& OldLocation, const FVector& OldVelocity) override;

	// Movement Settings Override
	virtual void PhysWalking(float deltaTime, int32 Iterations) override;
	virtual float GetMaxAcceleration() const override;
	virtual float GetMaxBrakingDeceleration() const override;

	// Movement Settings Variables
	UPROPERTY()
	uint8 bRequestMovementSettingsChange = 1;

	UPROPERTY()
	EALSGait AllowedGait = EALSGait::Walking;

	UPROPERTY(BlueprintReadOnly, Category = "ALS|Movement System")
	FALSMovementSettings CurrentMovementSettings;

	// Set Movement Curve (Called in every instance)
	float GetMappedSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetMovementSettings(FALSMovementSettings NewMovementSettings);

	// Set Max Walking Speed (Called from the owning client)
	UFUNCTION(BlueprintCallable, Category = "Movement Settings")
	void SetAllowedGait(EALSGait NewAllowedGait);

	UFUNCTION(Reliable, Server, Category = "Movement Settings")
	void Server_SetAllowedGait(EALSGait NewAllowedGait);


	//~ IALSGravityMovementInterface Functions	~//

	UPROPERTY(BlueprintAssignable, Category="Gravity")
	FOnGravityDirectionChanged OnGravityDirectionChanged;
	// /** Called after GravityDirectionMode (or related data) has changed.
	//  * @note Can be overriden
	//  * @param OldGravityDirectionMode - previous value of GravityDirectionMode
	//  * @param CurrentGravityDirectionMode - current value of GravityDirectionMode */
	// UFUNCTION(BlueprintImplementableEvent, Category="GravityCharacterMovement")
	// void OnGravityDirectionChanged(EGravityDirectionMode OldGravityDirectionMode, EGravityDirectionMode CurrentGravityDirectionMode);


	virtual void SetNewGravityScale(const float NewGravity) override;
	virtual void ResetGravityScale() override;

	/** Sets a new fixed gravity direction.
	 * @note It can be influenced by GravityScale
	 * @param NewFixedGravityDirection - new fixed gravity direction, assumes it is normalized  */
	virtual void SetFixedGravityDirection(const FVector& NewFixedGravityDirection) override;
	
	/** Sets a new gravity direction determined by closest spline tangent.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityActor - Actor that provides a spline */
	virtual void SetSplineTangentGravityDirection(AActor* NewGravityActor) override;
	
	/** Sets a new point which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityPoint - new point which gravity direction points to */
	virtual void SetPointGravityDirection(const FVector& NewGravityPoint) override;
	
	/** Sets a new point which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityActor - Actor that provides its location as gravity point */
	virtual void SetPointGravityDirectionFromActor(AActor* NewGravityActor) override;

	/** Sets a new infinite line which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityLineStart - a point that belongs to the infinite line
	 * @param NewGravityLineEnd - another point that belongs to the infinite line */
	virtual void SetLineGravityDirection(const FVector& NewGravityLineStart, const FVector& NewGravityLineEnd) override;
	
	/** Sets a new segment line which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravitySegmentStart - start point of the segment line
	 * @param NewGravitySegmentEnd - end point of the segment line */
	virtual void SetSegmentGravityDirection(const FVector& NewGravitySegmentStart, const FVector& NewGravitySegmentEnd) override;
	
	/** Sets a new spline which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityActor - Actor that provides a spline */
	virtual void SetSplineGravityDirection(AActor* NewGravityActor) override;

	/** Sets a new infinite plane which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityPlaneBase - a point that belongs to the plane
	 * @param NewGravityPlaneNormal - normal of the plane, assumes it is normalized */
	virtual void SetPlaneGravityDirection(const FVector& NewGravityPlaneBase, const FVector& NewGravityPlaneNormal) override;


	/** Sets a new infinite plane determined by closest spline point and spline
	 * up vector which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityActor - Actor that provides a spline */
	virtual void SetSplinePlaneGravityDirection(AActor* NewGravityActor) override;

	/** Sets a new axis-aligned box which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityBoxOrigin - origin of the box
	 * @param NewGravityBoxExtent - extents of the box */
	virtual void SetBoxGravityDirection(const FVector& NewGravityBoxOrigin, const FVector& NewGravityBoxExtent) override;

	/** Sets a new axis-aligned box which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityActor - Actor that provides its collision bounding box as gravity target */
	virtual void SetBoxGravityDirectionFromActor(AActor* NewGravityActor) override;

	/** Sets a new collision geometry which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityActor - Actor that owns the PrimitiveComponent that has collision geometry */
	virtual void SetCollisionGravityDirection(AActor* NewGravityActor) override;

	/** Returns true if the character is in the 'Walking' movement mode. */
	virtual bool GetIsWalking() const override;

	/** Queue a pending launch with velocity LaunchVel. */
	virtual void LaunchCharacter(const FVector& LaunchVel) override;
	

	//~ IALSGravityMovementInterface Functions	~//

	/**
	 * Update values related to gravity.
	 */
	virtual void UpdateGravity();

	/**
	 * Sends gravity data from server to clients.
	 */
	virtual void ReplicateGravityToClients();

	/**
	 * If true and a floor is found, rotate gravity direction and align it to floor base.
	 * @note For "Fixed" gravity mode, gravity direction is set to reverse floor normal vector
	 * @note For "Point" gravity mode, gravity direction points to base's location
	 * @note For "Box" gravity mode, gravity direction points to base's bounding box
	 * @note For "Collision" gravity mode, gravity direction points to base's collision geometry
	 */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,BlueprintSetter=SetAlignGravityToBase,Category="GravityCharacterMovement")
	uint32 bAlignGravityToBase:1;

	/**
	 * Sets a new state for bAlignGravityToBase flag.
	 * @param bNewAlignGravityToBase - new value for bAlignGravityToBase flag
	 */
	UFUNCTION(BlueprintSetter,Category="Pawn|Components|GravityCharacterMovement")
	virtual void SetAlignGravityToBase(bool bNewAlignGravityToBase);



protected:
	
	/**
	 * Enables bAlignGravityToBase flag for clients.
	 */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastEnableAlignGravityToBase();

	/**
	 * Disables bAlignGravityToBase flag for clients.
	 */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastDisableAlignGravityToBase();
	
	/**
	 * Replicates gravity scale factor to clients.
	 * @param NewGravityScale - new gravity scale factor
	 */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastSetGravityScale(const float NewGravityScale);

	
	/** Replicates a new fixed gravity direction to clients.
	* @param NewFixedGravityDirection - new fixed gravity direction, assumes it is normalized */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastSetFixedGravityDirection(const FVector& NewFixedGravityDirection);

	/** Replicates a new spline gravity direction to clients.
	* @param NewGravityActor - Actor that provides a spline */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastSetSplineTangentGravityDirection(AActor* NewGravityActor);

	/** Replicates a new gravity point to clients.
	 * @param NewGravityPoint - new point which gravity direction points to */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastSetPointGravityDirection(const FVector& NewGravityPoint);

	/** Replicates a new gravity point to clients.
	 * @param NewGravityActor - Actor that provides its location as gravity point */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastSetPointGravityDirectionFromActor(AActor* NewGravityActor);

	/** Replicates a new infinite line for gravity to clients.
	 * @param NewGravityLineStart - a point that belongs to the infinite line
	 * @param NewGravityLineEnd - another point that belongs to the infinite line */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastSetLineGravityDirection(const FVector& NewGravityLineStart, const FVector& NewGravityLineEnd);

	/** Replicates a new segment line for gravity to clients.
	 * @param NewGravitySegmentStart - start point of the segment line
	 * @param NewGravitySegmentEnd - end point of the segment line */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastSetSegmentGravityDirection(const FVector& NewGravitySegmentStart, const FVector& NewGravitySegmentEnd);

	/** Replicates a new spline for gravity to clients.
	* @param NewGravityActor - Actor that provides a spline */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastSetSplineGravityDirection(AActor* NewGravityActor);

	
	/** Replicates a new infinite plane for gravity to clients.
	 * @param NewGravityPlaneBase - a point that belongs to the plane
	 * @param NewGravityPlaneNormal - normal of the plane, assumes it is normalized */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastSetPlaneGravityDirection(const FVector& NewGravityPlaneBase, const FVector& NewGravityPlaneNormal);

	/** Replicates a new infinite plane determined by closest spline point for
	 * gravity to clients.
	 * @param NewGravityActor - Actor that provides a spline */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastSetSplinePlaneGravityDirection(AActor* NewGravityActor);

	
	/** Replicates a new axis-aligned box for gravity to clients.
	 * @param NewGravityBoxOrigin - origin of the box
	 * @param NewGravityBoxExtent - extents of the box */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastSetBoxGravityDirection(const FVector& NewGravityBoxOrigin, const FVector& NewGravityBoxExtent);

	/** Replicates a new axis-aligned box for gravity to clients.
	 * @param NewGravityActor - Actor that provides its collision bounding box as gravity target */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastSetBoxGravityDirectionFromActor(AActor* NewGravityActor);

	/** Replicates a new collision geometry for gravity to clients.
	 * @param NewGravityActor - Actor that owns the PrimitiveComponent that has collision geometry */
	UFUNCTION(NetMulticast,Reliable)
	virtual void MulticastSetCollisionGravityDirection(AActor* NewGravityActor);

	/** Called after GravityDirectionMode (or related data) has changed.
	 * @param OldGravityDirectionMode - previous value of GravityDirectionMode */
	void GravityDirectionChanged(const EGravityDirectionMode OldGravityDirectionMode);


	/**
	 * Asks if gravity data should be replicated from server to clients.
	 * @return true if gravity data should be replicated
	*/
	virtual bool GetShouldReplicateGravity() const;


	//~		Gravity Vars	~//
	
	/** Mode that determines direction of gravity. */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly,Category="GravityCharacterMovement")
	EGravityDirectionMode GravityDirectionMode;
	
	/** Stores information that determines direction of gravity. */
	UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="GravityCharacterMovement")
	FVector GravityVectorA;

	/** Stores additional information that determines direction of gravity. */
	UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="GravityCharacterMovement")
	FVector GravityVectorB;
	
	/** Optional Actor that determines direction of gravity. */
	UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly,Category="GravityCharacterMovement")
	TObjectPtr<AActor> GravityActor;

	/**
	 * Stores last known value of GravityScale.
	 * @see GravityScale
	*/
	float OldGravityScale;

	
	/** If true, gravity direction changed and needs to be replicated. */
	uint32 bDirtyGravityDirection:1;

	/** If true, gravity data isn't replicated from server to clients. */
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="GravityCharacterMovement")
	uint32 bDisableGravityReplication:1;


	//~		Gravity Vars	~//


};
