// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Boss/GTWBoss.h"

#include "Controllers/GTWBoss2PAIController.h"
#include "Actor/GTWStartMoveSplineActor.h"
#include "Actor/Gimmicks/GTWLightningGimmick.h"
#include "Actor/Gimmicks/GTWRoadblockGimmick.h"
#include "Actor/Gimmicks/GTWMeteorGimmick.h"
#include "Actor/Gimmicks/GTWSwordWaveGimmick.h"


#include "Components/CapsuleComponent.h"
#include <Animation/AnimInstance.h>
#include <Components/SplineComponent.h>
#include <Components/BoxComponent.h>

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

	// 박스 콜리전 추가. 
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	// 루트 컴포넌트 아래에 추가
	BoxCollision->SetupAttachment(GetRootComponent());
	BoxCollision->SetBoxExtent(FVector(0.7f, 2.5f, 2.8f));
	BoxCollision->SetCollisionProfileName(TEXT("Custom"));

}

void AGTWBoss::PlaySwordWave()
{
	// 캐릭터 애니메이션 실행
	PlayAttackAnim(SwordAttack);

	// 검기 액터 가져오기
	FActorSpawnParameters SpawnParams;
	GetWorld()->SpawnActor<AGTWSwordWaveGimmick>(
		SwordWaveClass,
		GetActorLocation() + GetActorForwardVector() * 100.0f + GetActorRightVector() * FMath::RandRange(-450, 450),
		GetActorRotation(),
		SpawnParams
	);

	// 태스크 실행 후 결과 반환
	UE_LOG(LogTemp, Log, TEXT("Start SwordWave"));
}

void AGTWBoss::PlaySpawnBall()
{
	// 캐릭터 애니메이션 실행 
	PlayAttackAnim(BallAttack);

	// 운석 액터 생성. 
	FActorSpawnParameters SpawnParams;
	GetWorld()->SpawnActor<AGTWMeteorGimmick>(
		MeteorClass,
		GetActorLocation() + GetActorForwardVector() * 2500.0f + GetActorRightVector() * FMath::RandRange(-450, 450),
		GetActorRotation(),
		SpawnParams
	);
}

void AGTWBoss::PlayLightning()
{
	PlayAttackAnim(LightningAttack);

	FActorSpawnParameters SpawnParams;
	GetWorld()->SpawnActor<AGTWLightningGimmick>(
		LightningClass,
		GetActorLocation() + GetActorForwardVector() * 2000.0f + GetActorRightVector() * FMath::RandRange(-450, 450),
		GetActorRotation(),
		SpawnParams
	);

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
	// 현재 스플라인 배열의 Tick을 비활성화
	BossSplines[CurrentSplineNum]->SetActorTickEnabled(false);

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

