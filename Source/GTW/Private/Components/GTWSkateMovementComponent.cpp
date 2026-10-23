// Fill out your copyright notice in the Description page of Project Settings.


#include "../../Public/Components/GTWSkateMovementComponent.h"
#include "../../Public/Characters/GTWPlayerCharacter.h"

#include <Engine/HitResult.h>
#include <Stats/Stats.h>
#include <GameFramework/Character.h>
#include <Components/CapsuleComponent.h>

UGTWSkateMovementComponent::UGTWSkateMovementComponent()
	: Super()
{
	GravityScaleForAcceleration = 0.05f;
}

void UGTWSkateMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	GTWPlayerCharacter = Cast<AGTWPlayerCharacter>(GetOwner());	
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

		// Don't change rotation if there is no acceleration.
		return CurrentRotation;
	}
	// Rotate toward direction of velocity.
	return Velocity.GetSafeNormal().Rotation();
}

void UGTWSkateMovementComponent::PhysicsRotation(float DeltaTime)
{
	// 바닥 법선 벡터
	FVector FloorNormalVector = GetImpactNormal();

	if (1 - FloorNormalVector.Z > KINDA_SMALL_NUMBER)
	{
		// 고점을 찍으면 
		if ((GTWPlayerCharacter->GetVelocity().X * FloorNormalVector.X
			+ GTWPlayerCharacter->GetVelocity().Y * FloorNormalVector.Y)
			> 0)
		{
			// 캐릭터 정면 벡터
			FVector MeshRightVector = GetMeshRightVector();

			// 중력 방향 결정 벡터
			// 법선 벡터에서 Z값을 90도 아래로 회전
			FVector SlopeGravity = FloorNormalVector - FVector(0, 0, 1);

			// 중력 추가 계산
			Acceleration += (-GetGravityZ()) * SlopeGravity;
		}
	}

	Super::PhysicsRotation(DeltaTime);
}

FVector UGTWSkateMovementComponent::GetMeshRightVector() const
{
	return GTWPlayerCharacter->GetMesh()->GetRightVector();
}

bool UGTWSkateMovementComponent::IsCustomMovementMode(ECustomMovement IsCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == IsCustomMovementMode;
}
