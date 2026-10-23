// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GTWSkateMovementComponent.generated.h"



UENUM(BlueprintType)
enum ECustomMovement
{
	CMOVE_Slide UMETA(DisplayName = "Slide"),
};



/**
 * 
 */
UCLASS()
class GTW_API UGTWSkateMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UGTWSkateMovementComponent();
	
	virtual void InitializeComponent() override; 

	// 상속
public:
	// TODO: 기존 로직에서 반환 전에 경사 계산 추가하기.  
	virtual FRotator ComputeOrientToMovementRotation(
		const FRotator& CurrentRotation,
		float DeltaTime,
		FRotator& DeltaRotation) const override;


	virtual void PhysicsRotation(float DeltaTime) override;

public:
	// 바닥 법선 벡터 구하기
	FVector GetImpactNormal() const
	{
		return CurrentFloor.HitResult.ImpactNormal;
	}

	FVector GetMeshRightVector() const;



public:
	// 커스텀 무브 확인 함수
	UFUNCTION(BlueprintPure)
	bool IsCustomMovementMode(ECustomMovement IsCustomMovementMode) const;

protected:
	// 오너
	class AGTWPlayerCharacter* GTWPlayerCharacter;

	// 중력 가속 스케일 계수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gravity)
	float GravityScaleForAcceleration;
};
	