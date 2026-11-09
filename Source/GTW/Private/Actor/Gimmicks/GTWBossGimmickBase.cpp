// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmicks/GTWBossGimmickBase.h"
#include <Components/SceneComponent.h>

void AGTWBossGimmickBase::PlayBurstFX(EGimmickFXTypes Type, const FTransform& WorldTransform)
{
	if (bIsEndingPlay || IsActorBeingDestroyed())
	{
		return;
	}

	OnPlayBurstFX(Type, WorldTransform);
}

void AGTWBossGimmickBase::StartLoopingFX(EGimmickFXTypes Type, USceneComponent* AttachTo)
{
	if (bIsEndingPlay || IsActorBeingDestroyed() || !IsValid(AttachTo))
	{
		return;
	}

	if (ActiveLoopingFX.Contains(Type))
	{
		return;
	}

	ActiveLoopingFX.Add(Type);
	OnStartLoopingFX(Type, AttachTo);
}

void AGTWBossGimmickBase::StopLoopingFX(EGimmickFXTypes Type)
{
	if (ActiveLoopingFX.Remove(Type) == 0)
	{
		return;
	}

	OnStopLoopingFX(Type);
}

void AGTWBossGimmickBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	bIsEndingPlay = true;

	const TArray<EGimmickFXTypes> RemainingFX = ActiveLoopingFX.Array();
	ActiveLoopingFX.Reset();

	for (const EGimmickFXTypes Type : RemainingFX)
	{
		OnStopLoopingFX(Type);
	}

	Super::EndPlay(EndPlayReason);
}
