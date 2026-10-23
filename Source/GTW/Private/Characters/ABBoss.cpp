// Fill out your copyright notice in the Description page of Project Settings.

#include <Animation/AnimInstance.h>
#include "Characters/ABBoss.h"
#include "AI/ABAIController.h"

// Sets default values
AABBoss::AABBoss()
{
	// 메시 컴포넌트 연결
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> BossMesh(
		TEXT("/Game/Characters/Boss/Mesh/SKM_Boss.SKM_Boss")
	);
	if (BossMesh.Succeeded())
		GetMesh()->SetSkeletalMesh(BossMesh.Object);


	// 매시 컴포넌트 앞쪽 보게 설정
	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.0f, 0.0f, -88.0f),
		FRotator(0.0f, -90.0f, 0.0f)
	);

	// 애니메이션 에셋 연결
	static ConstructorHelpers::FClassFinder<UAnimInstance> BossAnim(
		TEXT("/Game/Characters/Boss/Animation/ABP_P2Boss.ABP_P2Boss_C")
	);

	if (BossAnim.Succeeded())
		GetMesh()->SetAnimInstanceClass(BossAnim.Class);

	// AIController 연결
	AIControllerClass = AABAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

// Called when the game starts or when spawned
void AABBoss::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AABBoss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AABBoss::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

