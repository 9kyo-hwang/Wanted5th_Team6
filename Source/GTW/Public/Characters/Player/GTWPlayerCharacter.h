// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include <Interfaces/GTWWidgetInterface.h>

#include "GTWPlayerCharacter.generated.h"


UCLASS()
class GTW_API AGTWPlayerCharacter : public ACharacter
									,public IGTWWidgetInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGTWPlayerCharacter(const FObjectInitializer& ObjectInitializer);

public:
	// 틱에 호출될 경사 이동 함수
	void MoveSlope(const FVector& Normal);

	// 중력을 적용하는 함수
	void ActiveGravity(const bool bIsGround, const FVector& Normal);

	// 법선 벡터로부터 중력이 적용될 벡터를 계산하는 함수
	FVector NormalToGravity(const FVector& Normal);

	// 상속
protected:
	// 위젯과 상호작용할 인터페이스
	virtual void SetSpeedWidget(class UGTWSpeedWidget* InSpeedWidget) override;

protected:
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// 캐릭터 무브먼트 컴포넌트 
	// 만약 베이스로 나뉘어야 한다면 이 부분은 자손클래스로 이전
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	TObjectPtr<class UGTWSkateMovementComponent> SkateCharacterMovementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gravity)
	float NoTiltGravity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gravity)
	float TiltGravity;

};
