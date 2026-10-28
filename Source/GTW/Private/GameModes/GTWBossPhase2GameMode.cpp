// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/GTWBossPhase2GameMode.h"

#include <GameInstance/GTWGameInstance.h>

#include <GameFramework/Character.h>	// TEMP. 추후 보스 클래스 생기면 그걸로 체인지

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

void AGTWBossPhase2GameMode::InitGameplay(ACharacter* InBoss)
{
	if (CurrentState != EBossGameModeState::Ready)
	{
		Print(TEXT("[InitGameplay] Ready 상태가 아님"));
		return;
	}

	if (!IsValid(InBoss))
	{
		Print(TEXT("[InitGameplay] 유효하지 않은 보스"));
		return;
	}

	Boss = InBoss;

	// @TODO: 여기서는 맵에 존재하는 제어 대상들을 추가로 제어해야 하나?
}

void AGTWBossPhase2GameMode::StartPlay()
{
	Super::StartPlay();

	StartGameplay();
}

void AGTWBossPhase2GameMode::StartGameplay()
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

	if (!IsValid(Boss.Get()))
	{
		Print(TEXT("[StartGameplay] 보스가 제대로 세팅되지 않았음"));
		return;
	}

	CurrentState = EBossGameModeState::Play;

	K2_OnGameplayStarted();
}

void AGTWBossPhase2GameMode::OnGoalReached()
{
	if (CurrentState != EBossGameModeState::Play)
	{
		return;
	}

	ClearPhase();
}

void AGTWBossPhase2GameMode::ClearPhase()
{
	if (CurrentState != EBossGameModeState::Play)
	{
		return;
	}

	CurrentState = EBossGameModeState::Clear;
	StopGameplay();

	if (UGTWGameInstance* GameInstance = GetGameInstance<UGTWGameInstance>())
	{
		// @TEMP: 현재 StartNewGame()을 호출하는 경로가 없음 -> 임시로 BP로 열어주자.
		GameInstance->CompleteGame();
	}
	else
	{
		Print(TEXT("[Phase2] GameInstance를 찾을 수 없음"));
	}

	K2_OnPhaseFinished(CurrentState);
}

void AGTWBossPhase2GameMode::FailPhase()
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

void AGTWBossPhase2GameMode::StopGameplay()
{
	K2_OnGameplayStopped();
}
