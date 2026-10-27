// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/GTWAnimInstance.h"

UGTWAnimInstance::UGTWAnimInstance()
{
}

void UGTWAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	// 이 클래스의 소유자 무브먼트 가져오기
	//Owner = Cast<ACharacter>(GetOwningActor());
	//if(Owner)
		
}

void UGTWAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	//bIsAttack = 
}
