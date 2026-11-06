// Fill out your copyright notice in the Description page of Project Settings.


#include <Components/Player/GTWSkateMovementComponent.h>
#include <Characters/Player/GTWPlayerCharacter.h>

#include <Engine/HitResult.h>
#include <GameFramework/Character.h>
#include <Components/CapsuleComponent.h>
#include <CharacterMovementComponentAsync.h>
#include "Kismet/KismetSystemLibrary.h"

UGTWSkateMovementComponent::UGTWSkateMovementComponent()
	: Super()
{
	NormalCount = 0;
	DefaultSpeed = 1800.0f;
	DownHillSpeed = 2500.0f;
	MaxWalkSpeed = DefaultSpeed;
	bIsMovingDownSlope = false;
}

void UGTWSkateMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	// 초기화
	GTWPlayerCharacter = Cast<AGTWPlayerCharacter>(GetCharacterOwner());

	NewImpactNormal = FVector(0.0f, 0.0f, 0.0f);
	OldImpactNormal = FVector(0.0f, 0.0f, 0.0f);
	OlderFloor = CurrentFloor;
	GTWVelocity = FVector(0.0f, 0.0f, 0.0f);

}

void UGTWSkateMovementComponent::SetGTWVelocity()
{
	GTWVelocity = Velocity;
	if (IsMovingOnGround())
	{
		GTWVelocity.Z = -(GTWVelocity.X * NewImpactNormal.X + GTWVelocity.Y * NewImpactNormal.Y) / NewImpactNormal.Z;
	}
	else
	{
		GTWVelocity.Z = Velocity.Z;
	}

	// m/s -> km/h
	int GTWSpeed = GTWVelocity.Length() / 100000 * 3600;

	OnSpeedChanged.Broadcast(GTWSpeed);
}


FRotator UGTWSkateMovementComponent::ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation) const
{
	// Rotate toward direction of velocity.
	// 기존 가속으로 진행하던 적용하던 회전을 속도를 기반으로 회전하도록 변경함.
	// -> 회전이 진행되는 시점을 가속 적용 순간에서 속도 변환 시점으로 변환한 것.
	if (Velocity.SizeSquared2D() < KINDA_SMALL_NUMBER)
	{
		return CurrentRotation;
	}
	return Velocity.GetSafeNormal().Rotation();
}


void UGTWSkateMovementComponent::PhysicsRotation(float DeltaTime)
{
	// 바닥 법선 벡터
	NewImpactNormal = GetNewImpactNormal();
	OldImpactNormal = GetOldImpactNormal();
		
	FVector FloorNormalVector = NewImpactNormal;

	if (1 - FloorNormalVector.Z > KINDA_SMALL_NUMBER)
	{
		// 고점을 찍으면 회전을 위해 가속 추가
		if ((GTWPlayerCharacter->GetVelocity().X * FloorNormalVector.X
			+ GTWPlayerCharacter->GetVelocity().Y * FloorNormalVector.Y)
			> 0)
		{
			// 캐릭터 정면 벡터
			FVector MeshRightVector = GetMeshRightVector();

			// 중력 방향 결정 벡터
			// 법선 벡터에서 Z값을 90도 아래로 회전
			FVector SlopeGravity = FloorNormalVector;
			if (FloorNormalVector.Z != 0)
			{
				SlopeGravity.Z = -(FloorNormalVector.X * FloorNormalVector.X
									+ FloorNormalVector.Y * FloorNormalVector.Y)
									/ FloorNormalVector.Z;
			}
			// 중력 추가 계산
			Acceleration += (-GetGravityZ()) * SlopeGravity;
		}
	}

	// 입력 혹은 속도에 의한 회전 적용
	Super::PhysicsRotation(DeltaTime);

	bool bIsNextSlopeUpHill = NewImpactNormal.Z < OldImpactNormal.Z;

	if (bIsNextSlopeUpHill)
	{
		AddImpulse(FVector(0.0f, 0.0f, 10.0f), true);
	}

	SetGTWVelocity();
	
	// 다음 경사가 완만한가?
	bool bIsNextSlopeDownhill = NewImpactNormal.Z > OldImpactNormal.Z;

	// 최대 속도가 기본 속도인데 다음 경사가 더 완만하다면 
	// !bIsMovingDownSlope == 최대속도가 기본속도
	if (!bIsMovingDownSlope
		&& bIsNextSlopeDownhill)
	{
		bIsMovingDownSlope = true;

		MaxWalkSpeed = DownHillSpeed;
	}

	// 속도가 충분히 낮아졌는가?
	bool bCurrentSpeedIsSlow = Velocity.Length() < DefaultSpeed - 50.0f;

	if (bIsMovingDownSlope 
		&& bCurrentSpeedIsSlow)
	{
		bIsMovingDownSlope = false;

		MaxWalkSpeed = DefaultSpeed;
	}
	
	// OldFloor 업데이트
	OlderFloor = CurrentFloor;

	// 내적이 작다면 바닥의 옆면을 인식한 것으로 판단하여 업데이트를 하지 않음.
	if (FVector::DotProduct(NewImpactNormal, OldImpactNormal) < 0.1f)
	{
		NewImpactNormal = OldImpactNormal;

		if (NormalCount > 2)
		{
			NormalCount = 0;

			NewImpactNormal = OldImpactNormal = GetNewImpactNormal();
		}
		// 언덕을 넘어가는 등 상황이 발생 했을 때, 법선 벡터가 오래도록 업데이트 되지 않는 것을 방지.
		++NormalCount;
	}
	else
	{
		NormalCount = 0;
	}
}

void UGTWSkateMovementComponent::StartFalling(int32 Iterations, float remainingTime, float timeTick, const FVector& Delta, const FVector& subLoc)
{
	// start falling 
	const float DesiredDist = Delta.Size();
	const float ActualDist = ProjectToGravityFloor(UpdatedComponent->GetComponentLocation() - subLoc).Size();
	remainingTime = (DesiredDist < UE_KINDA_SMALL_NUMBER)
		? 0.0f
		: remainingTime + timeTick * (1.0f - FMath::Min(1.0f, ActualDist / DesiredDist));

	if (IsMovingOnGround())
	{
		// This is to catch cases where the first frame of PIE is executed, and the
		// level is not yet visible. In those cases, the player will fall out of the
		// world... So, don't set MOVE_Falling straight away.
		if (!GIsEditor || (GetWorld()->HasBegunPlay() && (GetWorld()->GetTimeSeconds() >= 1.0f)))
		{
			SetMovementMode(MOVE_Falling); //default behavior if script didn't change physics
		}
		else
		{
			// Make sure that the floor check code continues processing during this delay.
			bForceNextFloorCheck = true;
		}
	}

	NewImpactNormal.Normalize();


	// 경사 벗어났을 때 속도 역산
	if (NewImpactNormal.Z != 0)
	{
		Velocity.Z = -(Velocity.X * NewImpactNormal.X + Velocity.Y * NewImpactNormal.Y) / NewImpactNormal.Z;
	}
	 
	// 최대 속도 초기화.
	MaxWalkSpeed = DefaultSpeed;
	bIsMovingDownSlope = false;

	StartNewPhysics(remainingTime, Iterations);
}

bool UGTWSkateMovementComponent::ShouldCatchAir(
    const FFindFloorResult& OldFloor,
    const FFindFloorResult& NewFloor
)
{
	// 이전 및 현재 발판을 조사하여 모드를 Falling으로 바꿀지 말지 결정하는 함수. 
    if (Super::ShouldCatchAir(OldFloor, NewFloor))
    {
        return true;
    }

	// 밟을 수 있는 발 판인지 검증.
    if (!OldFloor.IsWalkableFloor() ||
        !NewFloor.IsWalkableFloor())
    {
        return false;
    }

    const float Speed = Velocity.Size2D();

	// 경사를 벗어나기 위한 속도가 500이상이어야 함.
    if (Speed < 500.0f)
    {
        return false;
    }

    const FVector Direction =
        Velocity.GetSafeNormal2D();

    const FVector OldNormal =
        OldFloor.HitResult.ImpactNormal;

    const FVector NewNormal =
        NewFloor.HitResult.ImpactNormal;

    // 진행 방향 기준 지형 기울기
    const float OldSlope =
        FVector::DotProduct(Direction, OldNormal);

    const float NewSlope =
        FVector::DotProduct(Direction, NewNormal);

    // 진행 방향으로 지면이 급격히 낮아지는가?
    const float SlopeChange = NewSlope - OldSlope;

    if (SlopeChange > 0.1f)
    {
        return true;
    }

    return false;
}

FVector UGTWSkateMovementComponent::GetMeshRightVector() const
{
	return GTWPlayerCharacter->GetMesh()->GetRightVector();
}