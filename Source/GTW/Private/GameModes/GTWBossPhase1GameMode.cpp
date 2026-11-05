// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/GTWBossPhase1GameMode.h"

#include <GTWGameInstance.h>
#include <GTWDebugHelper.h>
#include <Characters/Boss/GTWBoss.h>	// 1페는 다른 보스일 예정

#include <Kismet/GameplayStatics.h>

void AGTWBossPhase1GameMode::InitGameplay(AGTWBoss* InBoss)
{
	if (CurrentState != EBossGameModeState::Ready)
	{
		Debug::Print(TEXT("[InitGameplay] 상태가 Ready가 아님!"));
		return;
	}

	if (!IsValid(InBoss))
	{
		Debug::Print(TEXT("[InitGameplay] 보스가 유효하지 않음"));
		return;
	}

	/*
	* [x] 보스 참조 저장
	* [ ] 보스 명중 델리게이트 구독
	* [ ] 설정 전달(캐릭터 입력 막기 등?)
	* [ ] 연출 세팅은 Level BP에서??
	*/

	UnbindBossHitDelegate();

	Boss = InBoss;
	//BossHitDelegateHandle = Boss->OnValidHit.AddUObject(
	//	this,
	//	&ThisClass::OnHitBoss
	//);
}

void AGTWBossPhase1GameMode::StartPlay()
{
	Super::StartPlay();

	if (!IsValid(Boss.Get()))
	{
		Debug::Print(TEXT("[Phase1 GameMode] 보스 초기화가 되지 않음"));
		return;
	}

	if (OnIntroRequested.IsBound())
	{
		Debug::Print(TEXT("[Phase1 GameMode] 인트로 재생! 종료 시 StartGameplay"));
		OnIntroRequested.Broadcast();
	}
	else
	{
		StartGameplay();
	}
}

void AGTWBossPhase1GameMode::StartGameplay()
{
	/*
		[ ] 조건 확인
		[x] 유효타 = 0
		[x] 상태 = Play
		[ ] Boss->StartCombat()
	*/

	if (CurrentState != EBossGameModeState::Ready)
	{
		return;
	}

	if (!GetWorld() || !GetWorld()->HasBegunPlay())
	{
		Debug::Print(TEXT("[GameMode::StartGameplay] World가 BeginPlay를 하지 않았음!"));
		return;
	}

	if (!IsValid(Boss.Get()))
	{
		Debug::Print(TEXT("[GameMode::StartGameplay] 보스 제대로 세팅되지 않았음"));
		return;
	}

	CurrentHitCount = 0;
	CurrentState = EBossGameModeState::Play;

	//Boss->StartCombat();
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

void AGTWBossPhase1GameMode::UnbindBossHitDelegate()
{
	if (IsValid(Boss.Get()) && BossHitDelegateHandle.IsValid())
	{
		//Boss->OnValidHit.Remove(BossHitDelegateHandle);
	}

	BossHitDelegateHandle.Reset();
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
		Debug::Print(TEXT("[Phase1] GameInstance가 없음"));
	}

	if (OnOutroRequested.IsBound())
	{
		Debug::Print(TEXT("[Phase1 GameMode] 아웃트로 재생. 완료 후 레벨 변경 처리"));
		OnOutroRequested.Broadcast(CurrentState);
	}
	else
	{
		OpenNextLevel();
	}
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
		Debug::Print(TEXT("[Phase1] GameInstance가 없음"));
	}
	
	if (OnOutroRequested.IsBound())
	{
		Debug::Print(TEXT("[Phase1 GameMode] 아웃트로 재생. 완료 후 레벨 변경 처리"));
		OnOutroRequested.Broadcast(CurrentState);
	}
	else
	{
		OpenNextLevel();
	}
}

void AGTWBossPhase1GameMode::OpenNextLevel()
{
	if (bRequestedOpenNextLevel)
	{
		return;
	}

	// Clear나 Fail이 아니어야 함
	if (CurrentState != EBossGameModeState::Clear &&
		CurrentState != EBossGameModeState::Fail)
	{
		return;
	}

	const TSoftObjectPtr<UWorld>& LevelToOpen =
		CurrentState == EBossGameModeState::Clear
		? Phase2Level
		: Phase1Level;

	if (LevelToOpen.IsNull())
	{
		Debug::Print(TEXT("[Phase1 GameMode] 이동할 레벨이 지정되지 않음"));
		return;
	}

	bRequestedOpenNextLevel = true;
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelToOpen);
}

void AGTWBossPhase1GameMode::StopGameplay()
{
	UnbindBossHitDelegate();

	if (IsValid(Boss.Get()))
	{
		//Boss->StopCombat();
	}

	K2_OnGameplayStopped();
}

void AGTWBossPhase1GameMode::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	UnbindBossHitDelegate();

	if (IsValid(Boss.Get()))
	{
		//Boss->StopCombat();
	}

	Super::EndPlay(EndPlayReason);
}
