// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GTWSkateMovementComponent.generated.h"




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
	// 기존 로직에서 반환 전에 경사 계산 추가하기.  
	virtual FRotator ComputeOrientToMovementRotation(
		const FRotator& CurrentRotation,
		float DeltaTime,
		FRotator& DeltaRotation) const override;


	virtual void PhysicsRotation(float DeltaTime) override;

	virtual void StartFalling(int32 Iterations, float remainingTime, float timeTick, const FVector& Delta, const FVector& subLoc) override;

	virtual bool ShouldRemainVertical() const
	{
		// Always remain vertical when walking or falling.
		return IsMovingOnGround();
	}

	virtual void UpdateVelocityBeforeMovement(float DeltaSeconds) override;
	

public:
	// 바닥 법선 벡터 구하기
	FVector GetNewImpactNormal() const
	{
		return CurrentFloor.HitResult.ImpactNormal;
	}
	FVector GetOldImpactNormal() const
	{
		return OldFloor.HitResult.ImpactNormal;
	}

	// 캐릭터 정면 벡터 구하기
	FVector GetMeshRightVector() const;



protected:
	// 오너
	class AGTWPlayerCharacter* GTWPlayerCharacter;


	UPROPERTY(Category = Floor, VisibleInstanceOnly, BlueprintReadOnly)
	FFindFloorResult OldFloor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Floor)
	FVector NewImpactNormal;

	UPROPERTY()
	FVector OldImpactNormal;

	UPROPERTY()
	int32 NormalCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Movement: Walking")
	float MaxSpeed;

};
	