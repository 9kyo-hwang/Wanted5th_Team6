// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/GTWBoss.h"
#include "Controllers/GTWBossAIController.h"
#include <Animation/AnimInstance.h>

// Sets default values
AGTWBoss::AGTWBoss()
{
	// 메시 컴포넌트 연결
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BossMesh(
		TEXT("/Game/Assets/Boss/Meshes/SKM_Boss.SKM_Boss")
	);
	if (BossMesh.Succeeded())
	{
		GetMesh()->SetSkeletalMesh(BossMesh.Object);
	}


	// 매시 컴포넌트 앞쪽 보게 설정
	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.0f, 0.0f, -88.0f),
		FRotator(0.0f, -90.0f, 0.0f)
	);

	// 애니메이션 에셋 연결
	static ConstructorHelpers::FClassFinder<UAnimInstance> BossAnim(
		TEXT("/Game/GTW/Boss/ABP_P2Boss.ABP_P2Boss_C")
	);

	if (BossAnim.Succeeded())
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(BossAnim.Class);
	}

	// 검기 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> SwordAttackRef(
		TEXT("/Game/GTW/Boss/Animation/AM_SwordAttack.AM_SwordAttack")
	);
	if (SwordAttackRef.Succeeded())
	{
		SwordAttack = SwordAttackRef.Object;
	}

	// 번개 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> LightningAttackRef(
		TEXT("/Game/GTW/Boss/Animation/AM_LightAttack.AM_LightAttack")
	);
	if (LightningAttackRef.Succeeded())
	{ 
		LightningAttack = LightningAttackRef.Object;
	}

	// 공 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> BallAttackRef(
		TEXT("/Game/GTW/Boss/Animation/AM_BallAttack.AM_BallAttack")
	);
	if (BallAttackRef.Succeeded())
	{
		BallAttack = BallAttackRef.Object;
	}

	// AIController 연결
	AIControllerClass = AGTWBossAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AGTWBoss::PlaySwordWave()
{
	// 캐릭터 애니메이션 실행
	PlayAttackAnim(SwordAttack);

	// 검기 액터 가져오기


	// 태스크 실행 후 결과 반환
	UE_LOG(LogTemp, Log, TEXT("Start SwordWave"));
}

void AGTWBoss::PlaySpawnBall()
{
	// 캐릭터 애니메이션 실행 
	PlayAttackAnim(BallAttack);
}

void AGTWBoss::PlayLightning()
{
	PlayAttackAnim(LightningAttack);

}

void AGTWBoss::PlayAttackAnim(UAnimMontage* AttackMontage)
{
	// 캐릭터 애니메이션 실행
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		// 실행 중인 모든 몽타주 중지(블렌드 아웃 숫자 지정)
		AnimInstance->StopAllMontages(0.5f);
		// 몽타주 실행(1.0f: 재생 속도)
		AnimInstance->Montage_Play(AttackMontage, 1.0f);
	}
}

