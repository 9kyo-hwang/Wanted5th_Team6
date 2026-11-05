// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Boss/GTWBoss.h"

#include <Components/CapsuleComponent.h>
#include <Controllers/GTWBoss2PAIController.h>
#include <Actor/GTWStartMoveSplineActor.h>
#include <GTWDebugHelper.h>

#include <Animation/AnimInstance.h>
#include <Components/SplineComponent.h>

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
		TEXT("/Game/GTW/Characters/Boss/ABP_P2Boss.ABP_P2Boss_C")
	);

	if (BossAnim.Succeeded())
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(BossAnim.Class);
	}

	// 검기 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> SwordAttackRef(
		TEXT("/Game/GTW/Characters/Boss/Animation/AM_SwordAttack.AM_SwordAttack")
	);

	if (SwordAttackRef.Succeeded())
	{
		SwordAttack = SwordAttackRef.Object;
	}

	// 번개 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> LightningAttackRef(
		TEXT("/Game/GTW/Characters/Boss/Animation/AM_LightAttack.AM_LightAttack")
	);

	if (LightningAttackRef.Succeeded())
	{ 
		LightningAttack = LightningAttackRef.Object;
	}

	// 공 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> BallAttackRef(
		TEXT("/Game/GTW/Characters/Boss/Animation/AM_BallAttack.AM_BallAttack")
	);

	if (BallAttackRef.Succeeded())
	{
		BallAttack = BallAttackRef.Object;
	}

	// 캡슐 컴포넌트의 오브젝트 타입을 Boss(ECC_GameTraceChannel1)로 설정. 
	GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);

	// 보스 2페이즈 컨트롤러 클래스 설정. 
	AIControllerClass = AGTWBoss2PAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AGTWBoss::StartCombat()
{
	if (!GetController())
	{
		SpawnDefaultController();	// 2페이즈 AI컨트롤러 생성. 
	}

	AGTWBoss2PAIController* BossController = Cast<AGTWBoss2PAIController>(GetController());
	if (!BossController)
	{
		Debug::Print(TEXT("[P2 Boss::StartCombat] AI 컨트롤러 사용 불가"));
		return;
	}

	BossController->RunAI();
	StartSpline();
}

void AGTWBoss::StopCombat()
{
	// @TODO: AI, Spline 이동, 공격 활성화 같은 BT쪽 정리하기?
	AGTWBoss2PAIController* BossController = Cast<AGTWBoss2PAIController>(GetController());
	if (!BossController)
	{
		Debug::Print(TEXT("[P2 Boss::StopCombat] AI 컨트롤러 사용 불가"));
		return;
	}

	BossController->StopAI();
	// @TODO: Stop Spline 등
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

void AGTWBoss::StartSpline()
{
	AGTWStartMoveSplineActor* SplineActor = BossSplines[CurrentSplineNum];
	if (!IsValid(SplineActor))
	{
		return;
	}

	// 이동 시작. 
	SplineActor->K2_OnStartMovement();
}

void AGTWBoss::NextSpline()
{
	// 현재 스플라인 배열에서 ++;
	CurrentSplineNum++;

	if (!BossSplines.IsValidIndex(CurrentSplineNum))
	{
		return;
	}

	StartSpline();
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

