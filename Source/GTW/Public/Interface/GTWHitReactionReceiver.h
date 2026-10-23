// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GTWHitReactionReceiver.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGTWHitReactionReceiver : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GTW_API IGTWHitReactionReceiver
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void HitKnockbackObject(FVector HitDirection, float power) = 0;

	virtual void HitStunObject(float StunTime) = 0;
};
