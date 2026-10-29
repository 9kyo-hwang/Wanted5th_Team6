// Fill out your copyright notice in the Description page of Project Settings.

#include "../../../Public/Characters/Player/GTWPlayerCharacter.h"
#include "../../../Public/Components/Player/GTWSkateMovementComponent.h"

#include <Components/CapsuleComponent.h>

// Sets default values
AGTWPlayerCharacter::AGTWPlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 컴포넌트 획득
	SkateCharacterMovementComponent = Cast<UGTWSkateMovementComponent>(GetCharacterMovement());
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
	FVector GravityImpulse;

	// 중력 충격의 방향을 결정할 변수
	FVector GravityImpulseDirection;

	// 자연스러운 중력 연출을 위한 조정값
	float GravaityScale;

	if (bIsGround)
	{
		GravaityScale = -0.15f;

		// 법선 벡터로부터 중력 방향을 계산하는 함수
		GravityImpulseDirection = NormalToGravity(Normal);
	}
	else
	{
		// 공중에서는 기존 적용되는 중력이 있으므로 값을 줄임
		GravaityScale = -0.07f;

		GravityImpulseDirection = FVector(0, 0, -1);
	}

	// 중력 방향에 중력값 * 스케일 값을 곱
	GravityImpulse = GravityImpulseDirection * (GravityValue * GravaityScale);
	
	// 캐릭터 무브먼트 컴포넌트에게 충격 전달
	if (Normal.Z < 1 - KINDA_SMALL_NUMBER)
	{
		SkateCharacterMovementComponent->AddImpulse(GravityImpulse, true);	
	}
}

FVector AGTWPlayerCharacter::NormalToGravity(const FVector& Normal)
{
	FVector GravityDirection;

	if (Normal.Z < KINDA_SMALL_NUMBER)
	{
		// 천장이나 직각으로 붙어있을 때 중력 아래로 
		return GravityDirection = FVector(0, 0, -1);		
	}

	// 법선 벡터에서 XY를 유지한 채 법선벡터와 수직인 벡터 찾기
	GravityDirection.X = Normal.X;
	GravityDirection.Y = Normal.Y;
	//GravityDirection.Z = Normal.Z;

	// 값이 이게 아닌데... 동작이 잘돼요... ????????
	GravityDirection.Z = -1 *  Normal.Z / (Normal.X * Normal.X + Normal.Y * Normal.Y);

	GravityDirection.Normalize();

	return GravityDirection;
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

// Called to bind functionality to input
void AGTWPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

} 


 