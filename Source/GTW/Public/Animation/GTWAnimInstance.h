// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GTWAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class GTW_API UGTWAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UGTWAnimInstance();

protected:
	// 애니메이션 시작 시 실행
	virtual void NativeInitializeAnimation() override;
	// 애니메이션 업데이트시마다 실행
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
};
