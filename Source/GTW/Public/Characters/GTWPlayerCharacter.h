// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GTWPlayerCharacter.generated.h"


UCLASS()
class GTW_API AGTWPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGTWPlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// 충돌 검사를 위해 라인 트레이싱을 할 때, 자기 캐릭터는 해당 검사에서 제외하는 함수
	FCollisionQueryParams GetIgnoreCharacterParams() const;

	// 캐릭터 무브먼트 컴포넌트 
	// 만약 베이스로 나뉘어야 한다면 이 부분은 자손클래스로 이전
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	class UGTWSkateMovementComponent* SkateCharacterMovementComponent;


};
