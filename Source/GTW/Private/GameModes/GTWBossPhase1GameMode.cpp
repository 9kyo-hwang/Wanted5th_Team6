// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/GTWBossPhase1GameMode.h"

#include <GameInstance/GTWGameInstance.h>

#include <GameFramework/Character.h>  // TEMP. 추후 보스 클래스 생기면 그걸로 체인지

namespace
{
	static void Print(const FString& Message, FColor Color = FColor::MakeRandomColor(), int32 InKey = -1)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InKey, 7.f, Color, Message);
			UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
		}
	}
}

void AGTWBossPhase1GameMode::InitGameplay(ACharacter* InBoss, const TArray<AActor*>& InHitObjectSpawners)
{
	if (CurrentState != EBossGameModeState::Ready)
	{
		Print(TEXT("[InitGameplay] 상태가 Ready가 아님!"));
		return;
	}

	if (!IsValid(InBoss) || InHitObjectSpawners.IsEmpty())
	{
		Print(TEXT("[InitGameplay] 보스가 유효하지 않거나 스포너가 없음!"));
		return;
	}

	for (AActor* Spawner : InHitObjectSpawners)
	{
		if (!IsValid(Spawner))
		{
			Print(TEXT("[InitGameplay] 스포너가 유효하지 않음!"));
			return;
		}
	}

	Boss = InBoss;
	HitObjectSpawners.Reset();

	for (AActor* Spawner : InHitObjectSpawners)
	{
		// 혹시 모를 중복을 대비
		HitObjectSpawners.AddUnique(Spawner);
	}

	if (UGTWGameInstance* GameInstance = GetGameInstance<UGTWGameInstance>())
	{
		double SokobanClearTimeSeconds, ShootingClearTimeSeconds;
		GameInstance->TryGetMinigameClearTime(EMinigameType::Sokoban, SokobanClearTimeSeconds);
		GameInstance->TryGetMinigameClearTime(EMinigameType::Shooting, ShootingClearTimeSeconds);

		// @TODO: 난이도 세팅
	}
	
	// @TODO: Ready 상태에서는 주로 연출이 재생되니, 플레이어 입력을 차단했다가 풀어주는 기능 필요
}

void AGTWBossPhase1GameMode::StartPlay()
{
	Super::StartPlay();

	// 일단 여기서 바로 호출: 레벨에 배치된 액터들 BeginPlay 끝나면 시작
	StartGameplay();
}

// 연출이 있으면 그쪽에서, 없다면 StartPlay 마지막에 호출
void AGTWBossPhase1GameMode::StartGameplay()
{
	if (CurrentState != EBossGameModeState::Ready)
	{
		return;
	}

	if (!GetWorld() || !GetWorld()->HasBegunPlay())
	{
		Print(TEXT("[StartGameplay] World가 BeginPlay를 하지 않았음!"));
		return;
	}

	if (!IsValid(Boss.Get()) || HitObjectSpawners.IsEmpty())
	{
		Print(TEXT("[StartGameplay] 보스랑 스포너가 제대로 세팅되지 않았음"));
		return;
	}

	for (AActor* Spawner : HitObjectSpawners)
	{
		if (!IsValid(Spawner))
		{
			Print(TEXT("[StartGameplay] 이상한 스포너 객체가 껴있음!"));
			return;
		}
	}

	CurrentHitCount = 0;
	CurrentState = EBossGameModeState::Play;

	K2_OnGameplayStarted();
}

void AGTWBossPhase1GameMode::OnHitBoss()
{
	if (CurrentState != EBossGameModeState::Play)
	{
		return;
	}

	// 카운트 올렸을 때 횟수 충족했다면 Clear!
	if (++CurrentHitCount >= ClearHitCount)
	{
		ClearPhase();
	}
}

void AGTWBossPhase1GameMode::ClearPhase()
{
	if (CurrentState != EBossGameModeState::Play)
	{
		return;
	}

	CurrentState = EBossGameModeState::Clear;
	StopGameplay();
	
	if (UGTWGameInstance* GameInstance = GetGameInstance<UGTWGameInstance>())
	{
		GameInstance->SetBossPhase1Cleared();
	}
	else
	{
		Print(TEXT("[Phase1] GameInstance가 없음"));
	}

	K2_OnPhaseFinished(CurrentState);
}

void AGTWBossPhase1GameMode::FailPhase()
{
	if (CurrentState != EBossGameModeState::Play)
	{
		return;
	}

	CurrentState = EBossGameModeState::Fail;
	StopGameplay();

	if (UGTWGameInstance* GameInstance = GetGameInstance<UGTWGameInstance>())
	{
		GameInstance->ResetBossPhase1Cleared();
	}
	else
	{
		Print(TEXT("[Phase1] GameInstance가 없음"));
	}

	K2_OnPhaseFinished(CurrentState);
}

void AGTWBossPhase1GameMode::StopGameplay()
{
	K2_OnGameplayStopped();
}
