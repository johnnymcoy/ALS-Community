// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ALSGravityMovementInterface.generated.h"

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UALSGravityMovementInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ALSV4_CPP_API IALSGravityMovementInterface
{
	GENERATED_BODY()

public:

	//~ Gravity Functions	~//
	
	/** Sets a new fixed gravity direction.
	 * @note It can be influenced by GravityScale
	 * @param NewFixedGravityDirection - new fixed gravity direction, assumes it is normalized  */
	virtual void SetFixedGravityDirection(const FVector& NewFixedGravityDirection) = 0;
	
	/** Sets a new gravity direction determined by closest spline tangent.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityActor - Actor that provides a spline */
	virtual void SetSplineTangentGravityDirection(AActor* NewGravityActor) = 0;
	
	/** Sets a new point which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityPoint - new point which gravity direction points to */
	virtual void SetPointGravityDirection(const FVector& NewGravityPoint) = 0;
	
	/** Sets a new point which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityActor - Actor that provides its location as gravity point */
	virtual void SetPointGravityDirectionFromActor(AActor* NewGravityActor) = 0;

	/** Sets a new infinite line which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityLineStart - a point that belongs to the infinite line
	 * @param NewGravityLineEnd - another point that belongs to the infinite line */
	virtual void SetLineGravityDirection(const FVector& NewGravityLineStart, const FVector& NewGravityLineEnd) = 0;
	
	/** Sets a new segment line which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravitySegmentStart - start point of the segment line
	 * @param NewGravitySegmentEnd - end point of the segment line */
	virtual void SetSegmentGravityDirection(const FVector& NewGravitySegmentStart, const FVector& NewGravitySegmentEnd) = 0;
	
	/** Sets a new spline which gravity direction points to.LaunchVelLaunchVel) = 0;

	/** Sets a new infinite plane which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityPlaneBase - a point that belongs to the plane
	 * @param NewGravityPlaneNormal - normal of the plane, assumes it is normalized */
	virtual void SetPlaneGravityDirection(const FVector& NewGravityPlaneBase, const FVector& NewGravityPlaneNormal) = 0;

	/** Sets a new spline which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityActor - Actor that provides a spline */
	virtual void SetSplineGravityDirection(AActor* NewGravityActor) = 0;



	/** Sets a new infinite plane determined by closest spline point and spline
	 * up vector which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityActor - Actor that provides a spline */
	virtual void SetSplinePlaneGravityDirection(AActor* NewGravityActor) = 0;

	/** Sets a new axis-aligned box which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityBoxOrigin - origin of the box
	 * @param NewGravityBoxExtent - extents of the box */
	virtual void SetBoxGravityDirection(const FVector& NewGravityBoxOrigin, const FVector& NewGravityBoxExtent) = 0;

	/** Sets a new axis-aligned box which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityActor - Actor that provides its collision bounding box as gravity target */
	virtual void SetBoxGravityDirectionFromActor(AActor* NewGravityActor) = 0;

	/** Sets a new collision geometry which gravity direction points to.
	 * @note It can be influenced by GravityScale
	 * @param NewGravityActor - Actor that owns the PrimitiveComponent that has collision geometry */
	virtual void SetCollisionGravityDirection(AActor* NewGravityActor) = 0;

	/** Returns true if the character is in the 'Walking' movement mode. */
	virtual bool GetIsWalking() const = 0;

	/** Queue a pending launch with velocity LaunchVel. */
	virtual void LaunchCharacter(const FVector& LaunchVel) = 0;

	virtual void SetNewGravityScale(const float NewGravity) = 0;
	virtual void ResetGravityScale() = 0;

	//~ Gravity Functions	~//

};
