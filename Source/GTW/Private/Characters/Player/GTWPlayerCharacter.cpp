// Fill out your copyright notice in the Description page of Project Settings.

#include <Characters/Player/GTWPlayerCharacter.h>
#include <Components/Player/GTWSkateMovementComponent.h>
#include <UI/GTWSpeedWidget.h>
#include <UI/GTWTimerWidget.h>
#include <UI/GTWPauseMenuWidget.h>

#include <Components/CapsuleComponent.h>
#include <cassert>

// Sets default values
AGTWPlayerCharacter::AGTWPlayerCharacter(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<
		UGTWSkateMovementComponent>(
			ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	SkateCharacterMovementComponent =
		Cast<UGTWSkateMovementComponent>(GetCharacterMovement());

	assert(SkateCharacterMovementComponent);

	// 중력 기본 값 설정
	TiltGravity = -250.0f;
	NoTiltGravity = -450.0f;

	// 상호작용 기본 값 설정
	bCanKnockback = true;
	bCanStun = true;

	// 낙하 랙돌 속성
	FallingRagdollSpeed = 4000.0f;
	FallingRagdollAngle = 0.5f;
	FallingRagdollTime = 1.0f;

	// 랙돌 시간 / 기준
	LowForceRagdollDuration = 0.5f;
	MediumForceRagdollDuration = 1.0f;
	HighForceRagdollDuration = 1.3f;

	MediumKnockbackForceThreshold = 10.0f;
	HighKnockbackForceThreshold = 20.0f;

	bIsRagdoll = false;

	ElapsedTime = 0;
	bSecondFloorClear = false;
}

// Called when the game starts or when spawned
void AGTWPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AGTWPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedTime += DeltaTime * 100;

	//땅에 있는지 확인
	bool bIsGround = SkateCharacterMovementComponent->CurrentFloor.bWalkableFloor;

	//법선 벡터 가져오기
	FVector NormalVector = SkateCharacterMovementComponent->GetNewImpactNormal();

	// 경사 이동 
	if (bIsGround)
	{
		MoveSlope(NormalVector);
	}

	// 중력 적용
	ActiveGravity(bIsGround, NormalVector);

	OnTimeChanged.Broadcast(ElapsedTime);

	if (ElapsedTime > 1000 && !bSecondFloorClear)
	{
		OnClearLevelChanged.Broadcast(ELevelClear::SecondFloorClear, ElapsedTime);
		bSecondFloorClear = true;
	}

}

void AGTWPlayerCharacter::Landed(const FHitResult& Hit)
{	
	FVector NormailzeVelocity = SkateCharacterMovementComponent->Velocity;

	bool bIsUncontrollableSpeed = NormailzeVelocity.Length() > FallingRagdollSpeed;

	NormailzeVelocity.Normalize();

	bool bIsVertical = FVector::DotProduct(NormailzeVelocity, Hit.ImpactNormal) < FallingRagdollAngle;

	if (bIsUncontrollableSpeed && bIsVertical)
	{		
		if (FallingRagdollTime > 0.0f)
		{
			ReservationEndRagdoll(FallingRagdollTime);
		}
		if (!bIsRagdoll)
		{
			BeginRagdoll(FallingRagdollTime);
		}
	}
}


void AGTWPlayerCharacter::MoveSlope(const FVector& Normal)
{
	// 캐릭터의 정면 벡터
	FVector ForwardVector = GetMesh()->GetRightVector();

	// Z값 보정 
	if (Normal.Z != 0)
	{
		ForwardVector.Z = -(ForwardVector.X * Normal.X + ForwardVector.Y * Normal.Y) / Normal.Z;
	}

	// 법선 벡터와 정면 벡터를 기반으로 회전 생성
	FRotator NewRotation = FRotationMatrix::MakeFromZX(Normal, ForwardVector).Rotator();

	// 생성된 회전으로 회전 적용
	GetCapsuleComponent()->SetWorldRotation(NewRotation);
}

void AGTWPlayerCharacter::ActiveGravity(const bool bIsGround, const FVector& Normal)
{
	// 캐릭터 무브먼트 컴포넌트로부터 중력 가져오기
	float GravityValue = SkateCharacterMovementComponent->GetGravityZ();

	// 중력 충격을 담을 변수
	FVector GravityForceValue;

	// 중력 충격의 방향을 결정할 변수
	FVector GravityForceDirection;


	if (bIsGround)
	{
		// Walking 모드에서는 XY만 고려.
		// 중력이 아래로 향하는 방향이나, 수직항력에 의해서 변하는
		// 알짜힘(모든 힘)의 방향을 구하는 과정
		GravityForceDirection = Normal;
		GravityForceDirection.Z = GravityForceDirection.Z - 1;

		GravityForceValue = GravityForceDirection * (GravityValue * NoTiltGravity);
	}
	else
	{
		GravityForceDirection = FVector(0, 0, -1);

		// 중력 방향에 중력값 * 스케일 값을 곱
		GravityForceValue = GravityForceDirection * (GravityValue * TiltGravity);
	}

	// 캐릭터 무브먼트 컴포넌트에게 충격 전달
	bool bHasNoTilt = Normal.Z < 1 - KINDA_SMALL_NUMBER;

	if (bHasNoTilt)
	{
		SkateCharacterMovementComponent->AddForce(GravityForceValue);
	}
}

void AGTWPlayerCharacter::BeginRagdoll(float RagdollTime)
{
	if (RagdollTime <= 0.0f)
	{
		return;
	}

	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (nullptr == CharacterMesh)
	{
		return;
	}

	UCapsuleComponent* CharacterCapsuleComponent = GetCapsuleComponent();
	if (nullptr == CharacterCapsuleComponent)
	{
		return;
	}
	AController* CharacterController = GetController();
	if (nullptr == CharacterController)
	{
		return;
	}
	// 모두 검증되었을 때 랙돌 시작.

	// 랙돌 플래그
	bIsRagdoll = true;

	// 랙돌 실행
	CharacterMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	CharacterMesh->SetAllBodiesBelowSimulatePhysics(TEXT("pelvis"), true, false);
	CharacterMesh->SetAllBodiesBelowPhysicsBlendWeight(TEXT("pelvis"), 1.0f, false, false);

	// 회전 설정
	CharacterCapsuleComponent->SetWorldRotation(
		SkateCharacterMovementComponent->Velocity.Rotation());

	// 이동 + 입력 금지
	CharacterController->SetIgnoreMoveInput(true);
}

void AGTWPlayerCharacter::ReservationEndRagdoll(float RagdollTime)
{
	GetWorldTimerManager().SetTimer(
		RagdollTimerHandle,
		this,
		&AGTWPlayerCharacter::EndRagdoll,
		RagdollTime,
		false
	);

}

void AGTWPlayerCharacter::EndRagdoll()
{
	// 랙돌 시작 검증
	if (!bIsRagdoll)
	{
		return;
	}
	bIsRagdoll = false;
	
	// 랙돌 종료
	USkeletalMeshComponent* CharacterMesh = GetMesh();
	if (nullptr == CharacterMesh)
	{
		return;
	}
	CharacterMesh->SetAllBodiesSimulatePhysics(false);
	SkateCharacterMovementComponent->SetMovementMode(EMovementMode::MOVE_Walking);

	// 입력 복원
	AController* CharacterController = GetController();
	if (nullptr == CharacterController)
	{
		return;
	}
	CharacterController->SetIgnoreMoveInput(false);

}

void AGTWPlayerCharacter::SetSpeedWidget(UGTWSpeedWidget* InSpeedWidget)
{
	// 값 확인
	if (nullptr == InSpeedWidget)
	{
		return;
	}

	// 설정할 초기값
	int InitSpeed = 0;

	InSpeedWidget->UpdateSpeed(InitSpeed);

	SkateCharacterMovementComponent->OnSpeedChanged.AddUObject(InSpeedWidget, &UGTWSpeedWidget::UpdateSpeed);
}

void AGTWPlayerCharacter::SetTimerWidget(UGTWTimerWidget* InTimerWidget)
{
	// 값 확인
	if (nullptr == InTimerWidget)
	{
		return;
	}

	// 설정할 초기값
	int InitTimer = 0;

	InTimerWidget->UpdateTimer(InitTimer);

	// 임시 객체 
	OnTimeChanged.AddUObject(InTimerWidget, &UGTWTimerWidget::UpdateTimer);
	OnClearLevelChanged.AddUObject(InTimerWidget, &UGTWTimerWidget::UpdateClearLevel);
}

void AGTWPlayerCharacter::SetPauseMenuWidget(UGTWPauseMenuWidget* InSpeedWidget)
{
}

void AGTWPlayerCharacter::HitKnockbackObject(FVector HitDirection, float power)
{
	// 넉백 금지
	if (!bCanKnockback)
	{
		return;
	}
	
	bCanKnockback = false;

	//  충격 전달 방향 속도를 0으로 만들기 
	if (abs(HitDirection.X) > KINDA_SMALL_NUMBER)
	{
		SkateCharacterMovementComponent->Velocity.X = 0.0f;
	}
	if (abs(HitDirection.Y) > KINDA_SMALL_NUMBER)
	{
		SkateCharacterMovementComponent->Velocity.Y = 0.0f;
	}
	if (abs(HitDirection.Z) > KINDA_SMALL_NUMBER)
	{
		SkateCharacterMovementComponent->Velocity.Z = 0.0f;
	}

	// 스케일 전달
	FVector Pulse = HitDirection * power;

	// 충격 전달
	SkateCharacterMovementComponent->AddImpulse(Pulse, true);

	// 힘에 따라 랙돌 시간 결정
	float RagdollTime = LowForceRagdollDuration;
	if (power > HighKnockbackForceThreshold)
	{
		RagdollTime = HighForceRagdollDuration;
	}
	else if (power > MediumKnockbackForceThreshold)
	{
		RagdollTime = MediumForceRagdollDuration;
	}

	// 랙돌 실행
	if (RagdollTime > 0.0f)
	{
		ReservationEndRagdoll(RagdollTime);
	}
	if (!bIsRagdoll)
	{		
		BeginRagdoll(RagdollTime);
	}

	// 연속 피격을 막기 위한 무적 시간
	float InvincibilityDuration = 0.3f;

	GetWorldTimerManager().SetTimer(
		KnockbackTimerHandle,
		this,
		&AGTWPlayerCharacter::CanKnockback,
		InvincibilityDuration,
		false
	);
}

void AGTWPlayerCharacter::HitStunObject(float StunTime)
{
	// 스턴 금지
	if (!bCanStun || StunTime <= 0.0f)
	{
		return;
	}
	// 스턴 무적
	bCanStun = false;

	// 랙돌 실행
	ReservationEndRagdoll(StunTime);
	if (!bIsRagdoll)
	{		
		BeginRagdoll(StunTime);
	}

	// 연속적인 스턴 피격을 막기 위한 무적 시간
	float AddtionalInvincibilityDuration = 0.3f;

	GetWorldTimerManager().SetTimer(
		StunTimerHandle,
		this,
		&AGTWPlayerCharacter::CanStun,
		StunTime + AddtionalInvincibilityDuration,
		false
	);
}

void AGTWPlayerCharacter::ApplyJumpImpulse(float JumpPower)
{
	//  Z 방향 속도를 0으로 만들기 
	SkateCharacterMovementComponent->Velocity.Z = 0.0f;
	// 스케일 전달
	FVector Pulse = FVector(0.0f, 0.0f, 1.0f) * JumpPower;

	// 충격 전달
	SkateCharacterMovementComponent->AddImpulse(Pulse, true);
}

void AGTWPlayerCharacter::ApplyRepulsion(FVector RepulseDirection, float RepulsionPower)
{
	// XY축만 고려
	if (abs(RepulseDirection.X) > KINDA_SMALL_NUMBER)
	{
		SkateCharacterMovementComponent->Velocity.X = 0.0f;
	}
	if (abs(RepulseDirection.Y) > KINDA_SMALL_NUMBER)
	{
		SkateCharacterMovementComponent->Velocity.Y = 0.0f;
	}
	

	// 스케일 전달
	FVector Pulse = RepulseDirection * RepulsionPower;

	// 충격 전달
	SkateCharacterMovementComponent->AddImpulse(Pulse, true);
}











