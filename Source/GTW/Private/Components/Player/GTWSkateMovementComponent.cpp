// Fill out your copyright notice in the Description page of Project Settings.


#include "../../../Public/Components/Player/GTWSkateMovementComponent.h"
#include "../../../Public/Characters/Player/GTWPlayerCharacter.h"

#include <Engine/HitResult.h>
#include <GameFramework/Character.h>
#include <Components/CapsuleComponent.h>
#include "Kismet/KismetSystemLibrary.h"

UGTWSkateMovementComponent::UGTWSkateMovementComponent()
	: Super()
{
	GravityScaleForAcceleration = 0.05f;
	NormalCount = 0;
}

void UGTWSkateMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	GTWPlayerCharacter = Cast<AGTWPlayerCharacter>(GetOwner());	

	// 초기화
	NewImpactNormal = FVector(0.f, 0.f, 0.f);
	OldImpactNormal = FVector(0.f, 0.f, 0.f);
	OldFloor = CurrentFloor;

}

FRotator UGTWSkateMovementComponent::ComputeOrientToMovementRotation(const FRotator& CurrentRotation, float DeltaTime, FRotator& DeltaRotation) const
{
	if (Acceleration.SizeSquared() < UE_KINDA_SMALL_NUMBER)
	{
		// AI path following request can orient us in that direction (it's effectively an acceleration)
		if (bHasRequestedVelocity && RequestedVelocity.SizeSquared() > UE_KINDA_SMALL_NUMBER)
		{
			return RequestedVelocity.GetSafeNormal().Rotation();
		}

		return CurrentRotation; 
	}

	// Rotate toward direction of velocity.
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

	// OldFloor 업데이트
	OldFloor = CurrentFloor;

	// 내적이 작다면 바닥의 옆면을 인식한 것으로 판단하여 업데이트를 하지 않음.
	if (FVector::DotProduct(NewImpactNormal, OldImpactNormal) < 0.05) 
	{
		if (NormalCount > 100)
		{
			NormalCount = 0;
		}

		NewImpactNormal = OldImpactNormal;

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
		? 0.f
		: remainingTime + timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));

	NewImpactNormal.Normalize();

	if (IsMovingOnGround())
	{
		// This is to catch cases where the first frame of PIE is executed, and the
		// level is not yet visible. In those cases, the player will fall out of the
		// world... So, don't set MOVE_Falling straight away.
		if (!GIsEditor || (GetWorld()->HasBegunPlay() && (GetWorld()->GetTimeSeconds() >= 1.f)))
		{
			SetMovementMode(MOVE_Falling); //default behavior if script didn't change physics
		}
		else
		{
			// Make sure that the floor check code continues processing during this delay.
			bForceNextFloorCheck = true;
		}
	}

	Velocity.Z = -(Velocity.X * NewImpactNormal.X + Velocity.Y * NewImpactNormal.Y) / NewImpactNormal.Z;

	StartNewPhysics(remainingTime, Iterations);
}


FVector UGTWSkateMovementComponent::GetMeshRightVector() const
{
	return GTWPlayerCharacter->GetMesh()->GetRightVector();
}

bool UGTWSkateMovementComponent::IsCustomMovementMode(ECustomMovement IsCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == IsCustomMovementMode;
}
