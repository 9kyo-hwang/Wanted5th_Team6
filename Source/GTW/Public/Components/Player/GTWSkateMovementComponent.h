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

	virtual bool ShouldCatchAir(const FFindFloorResult& OldFloor, const FFindFloorResult& NewFloor) override;

	virtual void HandleImpact(
		const FHitResult& Hit,
		float TimeSlice,
		const FVector& MoveDelta
	) override;
	/// 게터
public:
	// 바닥 법선 벡터 구하기
	FORCEINLINE FVector GetNewImpactNormal() const
	{
		return CurrentFloor.HitResult.ImpactNormal;
	}
	FORCEINLINE FVector GetOldImpactNormal() const
	{
		return OlderFloor.HitResult.ImpactNormal;
	}

	FORCEINLINE int GetGTWSpeed() const
	{
		return GTWSpeed;
	}
	FORCEINLINE bool IsStop() const
	{
		return bIsStop;
	}
	FORCEINLINE bool StartSkate() const
	{
		return bStartSkate;
	}
	FORCEINLINE bool WasSkate() const
	{
		return bWasSkate;
	}
	// 세터
	FORCEINLINE void SetWasSkate() 
	{
		bWasSkate = bStartSkate;
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
	FFindFloorResult OlderFloor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Floor)
	FVector NewImpactNormal;

	UPROPERTY()
	FVector OldImpactNormal;

	UPROPERTY()
	int32 NormalCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement)
	FVector GTWVelocity;

	// 기본 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed)
	float DefaultSpeed;

	// 내리막 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Speed)
	float DownHillSpeed;

	// 내려오는지 
	UPROPERTY()
	bool bIsMovingDownSlope;
	
	UPROPERTY()
	int GTWSpeed;

	// 멈췄는지
	UPROPERTY()
	bool bIsStop;

	// 이동을 시작했는지
	UPROPERTY()
	bool bStartSkate;

	// 이동을 시작했는지
	UPROPERTY()
	bool bWasSkate;

	// 충돌 재진입 방지 시간
	UPROPERTY()
	double ImpactDeltaTime;

	// 충돌 후 소요 계산할 변수
	UPROPERTY()
	double LastImpactTime;
};
	