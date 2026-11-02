// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Gimmicks/GTWLightningSpawner.h"

#include <Actor/Gimmicks/GTWLightningGimmick.h>
#include <GTWDebugHelper.h>

#include <Algo/RandomShuffle.h>

// Sets default values
AGTWLightningSpawner::AGTWLightningSpawner()
{
	LightningClass = AGTWLightningGimmick::StaticClass();
}

int32 AGTWLightningSpawner::SpawnLightning(int32 SpawnCount)
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !IsValid(LightningClass.Get()))
	{
		Debug::Print(TEXT("[LightningSpawner] World 또는 번개 클래스가 유효하지 않음"));
		return 0;
	}

	if (SpawnPositions.IsEmpty() || !SpawnPositions.IsValidIndex(SpawnCount - 1))
	{
		Debug::Print(TEXT("[LightningSpawner] 스폰 지점이 없거나 개수가 유효하지 않음"));
		return 0;
	}

	TArray<FVector> ShuffledPositions = SpawnPositions;
	Algo::RandomShuffle(ShuffledPositions);

	int32 Count = 0;
	for (int32 i = 0; i < SpawnCount; ++i)
	{
		const FVector WorldSpawnLocation = GetActorTransform().TransformPosition(ShuffledPositions[i]);
		if (AGTWLightningGimmick* Lightning = GetWorld()->SpawnActor<AGTWLightningGimmick>(LightningClass, WorldSpawnLocation, FRotator::ZeroRotator))
		{
			++Count;
		}
		else
		{
			Debug::Print(FString::Printf(TEXT("[LightningSpawner] 번개 생성 실패: %s"), *WorldSpawnLocation.ToString()));
		}
	}

	return Count;
}
