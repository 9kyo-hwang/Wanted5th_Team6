// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Gimmicks/GTWMeteorSpawner.h"

#include <Actor/Gimmicks/GTWMeteorGimmick.h>
#include <GTWDebugHelper.h>

#include <Algo/RandomShuffle.h>

// Sets default values
AGTWMeteorSpawner::AGTWMeteorSpawner()
{
	MeteorClass = AGTWMeteorGimmick::StaticClass();
}

// Return: 실제로 생성된 운석 개수
int32 AGTWMeteorSpawner::SpawnMeteor(int32 SpawnCount)
{
	UWorld* World = GetWorld();
	if (!IsValid(World) || !IsValid(MeteorClass.Get()))
	{
		Debug::Print(TEXT("[MeteorSpawner] World 또는 운석 클래스가 유효하지 않음"));
		return 0;
	}

	if (SpawnPositions.IsEmpty() || !SpawnPositions.IsValidIndex(SpawnCount - 1))
	{
		Debug::Print(TEXT("[MeteorSpawner] 스폰 지점이 없거나 개수가 유효하지 않음"));
		return 0;
	}

	TArray<FVector> ShuffledPositions = SpawnPositions;
	Algo::RandomShuffle(ShuffledPositions);

	int32 Count = 0;
	for (int32 i = 0; i < SpawnCount; ++i)
	{
		const FVector WorldSpawnLocation = GetActorTransform().TransformPosition(ShuffledPositions[i]);
		if (AGTWMeteorGimmick* Meteor = GetWorld()->SpawnActor<AGTWMeteorGimmick>(MeteorClass, WorldSpawnLocation, FRotator::ZeroRotator))
		{
			++Count;
		}
		else
		{
			Debug::Print(FString::Printf(TEXT("[MeteorSpawner] 운석 생성 실패: %s"), *WorldSpawnLocation.ToString()));
		}
	}

	return Count;
}
