// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GTWSkateMovementComponent.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FOnSpeedChangedDelegate, int /*CurrentSpeed*/);


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

	// 오너가 접근해서 GTWVelocity를 가져갈 함수.
	void SetGTWVelocity();

	// 상속
private:
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


	/// 게터
public:
	// 바닥 법선 벡터 구하기
	FORCEINLINE FVector GetNewImpactNormal() const
	{
		return CurrentFloor.HitResult.ImpactNormal;
	}
	FORCEINLINE FVector GetOldImpactNormal() const
	{
		return OldFloor.HitResult.ImpactNormal;
	}

	// 캐릭터 정면 벡터 구하기
	FVector GetMeshRightVector() const;

	// 델리게이트
public:
	// 속도 변환 시 갱신
	FOnSpeedChangedDelegate OnSpeedChanged;

protected:
	// 오너
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	TObjectPtr<class AGTWPlayerCharacter> GTWPlayerCharacter;

	UPROPERTY(Category = Floor, VisibleInstanceOnly, BlueprintReadOnly)
	FFindFloorResult OldFloor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Floor)
	FVector NewImpactNormal;

	UPROPERTY()
	FVector OldImpactNormal;

	UPROPERTY()
	int32 NormalCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	FVector GTWVelocity;

};
	