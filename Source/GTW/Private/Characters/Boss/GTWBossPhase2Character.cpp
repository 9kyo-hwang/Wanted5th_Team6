// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/Boss/GTWBossPhase2Character.h"
#include <GTWDebugHelper.h>

#include <Controllers/GTWBoss2PAIController.h>
#include <Actor/GTWStartMoveSplineActor.h>
#include <Actor/Gimmicks/GTWLightningGimmick.h>
#include <Actor/Gimmicks/GTWRoadblockGimmick.h>
#include <Actor/Gimmicks/GTWMeteorGimmick.h>
#include <Actor/Gimmicks/GTWSwordWaveGimmick.h>

#include <Animation/AnimInstance.h>
#include <Components/SplineComponent.h>
#include <Components/CapsuleComponent.h>
#include <Components/BoxComponent.h>
#include <GameFramework/CharacterMovementComponent.h>

// Sets default values
AGTWBossPhase2Character::AGTWBossPhase2Character()
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
		ABPClass = BossAnim.Class;
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	}

	// 검기 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> SwordAttackRef(
		TEXT("/Game/GTW/Characters/Boss/Animation/AM_SwordAttack.AM_SwordAttack")
	);

	if (SwordAttackRef.Succeeded())
	{
		SwordWaveAttack = SwordAttackRef.Object;
	}

	// 번개 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> LightningAttackRef(
		TEXT("/Game/GTW/Characters/Boss/Animation/AM_LightAttack.AM_LightAttack")
	);

	if (LightningAttackRef.Succeeded())
	{ 
		LightningAttack = LightningAttackRef.Object;
	}

	// 운석(이전: 공) 몽타주 연결
	static ConstructorHelpers::FObjectFinder<UAnimMontage> MeteorAttackRef(
		TEXT("/Game/GTW/Characters/Boss/Animation/AM_BallAttack.AM_BallAttack")
	);

	if (MeteorAttackRef.Succeeded())
	{
		MeteorAttack = MeteorAttackRef.Object;
	}

	// 캡슐 컴포넌트의 오브젝트 타입을 Boss(ECC_GameTraceChannel1)로 설정. 
	GetCapsuleComponent()->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);

	// 박스 콜리전 추가. 
	DeathBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DeathBox"));
	// 루트 컴포넌트 아래에 추가
	DeathBox->SetupAttachment(GetRootComponent());
	DeathBox->SetBoxExtent(FVector(0.7f, 2.5f, 2.8f));
	DeathBox->SetCollisionProfileName(TEXT("Custom"));

	// 보스 2페이즈 컨트롤러 클래스 설정. 
	AIControllerClass = AGTWBoss2PAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AGTWBossPhase2Character::StartCombat()
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

	GetMesh()->SetAnimInstanceClass(ABPClass);
	BossController->RunAI();
	StartSpline();
	bCombatActive = true;
}

void AGTWBossPhase2Character::StopCombat()
{
	// @TODO: AI, Spline 이동, 공격 활성화 같은 BT쪽 정리하기?
	AGTWBoss2PAIController* BossController = Cast<AGTWBoss2PAIController>(GetController());
	if (!BossController)
	{
		Debug::Print(TEXT("[P2 Boss::StopCombat] AI 컨트롤러 사용 불가"));
		return;
	}

	bCombatActive = false;

	BossController->StopAI();
	// 현재 스플라인 배열의 Tick을 비활성화
	if (BossSplines.IsValidIndex(CurrentSplineNum))
	{
		BossSplines[CurrentSplineNum]->SetActorTickEnabled(false);
	}

	// Movement Mode를 MOVE_None으로 변경(중력/이동 정지)
	GetCharacterMovement()->DisableMovement();
	
	// ABP 정리. (시퀀스 시 애니메이션 정지시키기 위해)
	GetMesh()->SetAnimInstanceClass(nullptr);
}

void AGTWBossPhase2Character::StartSpline()
{
	AGTWStartMoveSplineActor* SplineActor = BossSplines[CurrentSplineNum];
	if (!IsValid(SplineActor))
	{
		return;
	}

	// 이동 시작. 
	SplineActor->K2_OnStartMovement();
}

void AGTWBossPhase2Character::NextSpline()
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

void AGTWBossPhase2Character::PlaySwordWaveMontage()
{
	PlayAttackMontage(SwordWaveAttack);

	SpawnSwordWaveGimmick();
}

void AGTWBossPhase2Character::PlayMeteorMontage()
{
	PlayAttackMontage(MeteorAttack);

	SpawnMeteorGimmick();
}

void AGTWBossPhase2Character::PlayLightningMontage()
{
	PlayAttackMontage(LightningAttack);

	SpawnLightningGimmick();
}

void AGTWBossPhase2Character::PlayAttackMontage(UAnimMontage* Montage)
{
	// 캐릭터 애니메이션 실행
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance)
	{
		return;
	}

	// 실행 중인 모든 몽타주 중지(블렌드 아웃 숫자 지정)
	AnimInstance->StopAllMontages(0.5f);

	// 몽타주 실행(1.0f: 재생 속도)
	if (AnimInstance->Montage_Play(Montage, 1.0f) <= 0.f)
	{
		return;
	}

	FOnMontageEnded OnMontageEnded;
	OnMontageEnded.BindUObject(this, &ThisClass::OnAttackMontageEnded);
	AnimInstance->Montage_SetEndDelegate(OnMontageEnded);
}

void AGTWBossPhase2Character::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (bInterrupted || !bCombatActive)
	{
		return;
	}

	// 불필요해짐
	//if (Montage == SwordWaveAttack)
	//{
	//	SpawnSwordWaveGimmick();
	//}
	//else if (Montage == MeteorAttack)
	//{
	//	SpawnMeteorGimmick();
	//}
	//else if (Montage == LightningAttack)
	//{
	//	SpawnLightningGimmick();
	//}
}

void AGTWBossPhase2Character::SpawnSwordWaveGimmick()
{
	// 검기 액터 가져오기
	FActorSpawnParameters SpawnParams;
	GetWorld()->SpawnActor<AGTWSwordWaveGimmick>(
		SwordWaveClass,
		//GetActorLocation() + GetActorForwardVector() * 100.0f + GetActorRightVector() * FMath::RandRange(-450, 450),
		CalculateGimmickSpawnLocation(SwordWaveSpawnSettings),
		GetActorRotation(),
		SpawnParams
	);
}

void AGTWBossPhase2Character::SpawnMeteorGimmick()
{
	// 운석 액터 생성. 
	FActorSpawnParameters SpawnParams;
	GetWorld()->SpawnActor<AGTWMeteorGimmick>(
		MeteorClass,
		//GetActorLocation() + GetActorForwardVector() * 2500.0f + GetActorRightVector() * FMath::RandRange(-450, 450),
		CalculateGimmickSpawnLocation(MeteorSpawnSettings),
		GetActorRotation(),
		SpawnParams
	);
}

void AGTWBossPhase2Character::SpawnLightningGimmick()
{
	FActorSpawnParameters SpawnParams;
	GetWorld()->SpawnActor<AGTWLightningGimmick>(
		LightningClass,
		//GetActorLocation() + GetActorForwardVector() * 2000.0f + GetActorRightVector() * FMath::RandRange(-450, 450),
		CalculateGimmickSpawnLocation(LightningSpawnSettings),
		GetActorRotation(),
		SpawnParams
	);
}

FVector AGTWBossPhase2Character::CalculateGimmickSpawnLocation(const FGTWBossGimmickSpawnSettings& Settings) const
{
	const float Offset = FMath::FRandRange(
		-Settings.HorizontalRandomRange,
		Settings.HorizontalRandomRange
	);

	return GetActorLocation() 
		+ GetActorForwardVector() * Settings.ForwardDistance 
		+ GetActorRightVector() * Offset;
}

