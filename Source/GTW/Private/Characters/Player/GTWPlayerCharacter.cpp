// Fill out your copyright notice in the Description page of Project Settings.

#include <Characters/Player/GTWPlayerCharacter.h>
#include <Components/Player/GTWSkateMovementComponent.h>
#include <UI/GTWSpeedWidget.h>

#include <Components/CapsuleComponent.h>

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

// Called when the game starts or when spawned
void AGTWPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 중력 기본 값 설정
	TiltGravity = -250.f;
	NoTiltGravity = -450.f;
}

// Called every frame
void AGTWPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	UE_LOG(LogTemp, Warning,
		TEXT("Mode=%d Ground=%d Normal=%s Velocity=%s Accel=%s"),
		static_cast<int32>(SkateCharacterMovementComponent->MovementMode),
		bIsGround,
		*NormalVector.ToString(),
		*SkateCharacterMovementComponent->Velocity.ToString(),
		*SkateCharacterMovementComponent->GetCurrentAcceleration().ToString());
}



