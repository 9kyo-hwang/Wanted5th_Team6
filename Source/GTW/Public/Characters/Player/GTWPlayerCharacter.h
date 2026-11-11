// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include <Interfaces/GTWWidgetInterface.h>
#include <Interfaces/GTWHitReactionReceiver.h>
#include <Interfaces/GTWInteractionGimmicksInterface.h>

#include "GTWPlayerCharacter.generated.h"

UCLASS()
class GTW_API AGTWPlayerCharacter : public ACharacter
									,public IGTWWidgetInterface
									,public IGTWHitReactionReceiver
									,public IGTWInteractionGimmicksInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AGTWPlayerCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Landed(const FHitResult& Hit) override;

	void PlayerWallImpact(int Speed);

	UFUNCTION(BlueprintCallable, Category = Hit)
	void ApplyKnockback(FVector HitDirection, float Power);
protected:
	// 틱에 호출될 경사 이동 함수
	void MoveSlope(const FVector& Normal);

	// 중력을 적용하는 함수
	void ActiveGravity(const bool bIsGround, const FVector& Normal);

	// 랙돌
protected:
	// 랙돌 시작 함수
	void BeginRagdoll(float RagdollTime);

	// 랙돌 종료 예약 함수
	void ReservationEndRagdoll(float RagdollTime);

	// 랙돌 종료 함수 랙돌 시작 시 타이머로 함수 실행 등록
	void EndRagdoll();

	// 상속
protected:
	// 위젯과 상호작용할 인터페이스
	virtual void SetSpeedWidget(class UGTWSpeedWidget* InSpeedWidget) override;
	
	// Hit 객체와 상호작용할  인터페이스
	virtual void HitKnockbackObject(FVector HitDirection, float power) override;
	virtual void HitStunObject(float StunTime) override;

	// Interact 객체와 상호작용할 인터페이스
	virtual void ApplyJumpImpulse(float JumpPower) override;
	virtual void ApplyRepulsion(FVector RepulseDirection, float RepulsionPower) override;

private:
	// 넉백 무적 시간 ON
	void CanKnockback()
	{
		bCanKnockback = true;
	}
	// 스턴 무적 시간 ON
	void CanStun()
	{
		bCanStun = true;
	}
	
	// 델리게이트 
protected:
	// 사운드 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Audio)
	TObjectPtr<class UGTWPlayerAudioComponent> SkateAudioComponent;
	// 이동 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Movement)
	TObjectPtr<class UGTWSkateMovementComponent> SkateCharacterMovementComponent;

	// 평지 중력
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gravity)
	float NoTiltGravity;

	// 경사 중력
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gravity)
	float TiltGravity;

	// 넉백 타이머
	UPROPERTY()
	FTimerHandle KnockbackTimerHandle;

	// 스턴 타이머
	UPROPERTY()
	FTimerHandle StunTimerHandle;

	// 랙돌 타이머
	UPROPERTY()
	FTimerHandle RagdollTimerHandle;

	// 넉백 무적
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Hit)
	bool bCanKnockback;

	// 스턴 무적
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Hit)
	bool bCanStun;

	// 낙하 시 적용할 랙돌 속성
	// 이 속력보다 큰 속력으로 떨어지면 랙돌
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ragdoll)
	float FallingRagdollSpeed;
	
	// 이 값이 가리키는 각도보다 크게 떨어지면 랙돌
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ragdoll)
	float FallingRagdollAngle;

	// 낙하 랙돌 시 실행 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ragdoll)
	float FallingRagdollTime;

	// 랙돌 시간 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ragdoll)
	float LowForceRagdollDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ragdoll)
	float MediumForceRagdollDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ragdoll)
	float HighForceRagdollDuration;

	// 랙돌 기준 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ragdoll)
	float MediumKnockbackForceThreshold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Ragdoll)
	float HighKnockbackForceThreshold;

	// 랙돌 재진입 방지 
	UPROPERTY()
	bool bIsRagdoll;


	// 클리어위젯 재진입 방지 
	UPROPERTY()
	bool bSokobanClear;

	// 충돌 소리나는 역치 값
	UPROPERTY()
	int HitThreshold;

	// 충돌 소리나는 역치 값
	UPROPERTY()
	int StrongHitThreshold;

};
