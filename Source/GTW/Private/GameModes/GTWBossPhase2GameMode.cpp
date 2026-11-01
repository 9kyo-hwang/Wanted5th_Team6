// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/GTWBossPhase2GameMode.h"

#include <GTWGameInstance.h>
#include <GTWDebugHelper.h>

#include <Characters/Boss/GTWBoss.h>
#include <Controllers/GTWBoss2PAIController.h>
#include <Characters/Player/GTWPlayerCharacter.h>

void AGTWBossPhase2GameMode::InitGameplay(AGTWBoss* InBoss)
{
	if (CurrentState != EBossGameModeState::Ready)
	{
		Debug::Print(TEXT("[InitGameplay] Ready 상태가 아님"));
		return;
	}

	if (!IsValid(InBoss))
	{
		Debug::Print(TEXT("[InitGameplay] 유효하지 않은 보스"));
		return;
	}

	// 보스 캐릭터 설정.
	Boss = InBoss;

	// 보스 2페이즈 컨트롤러 클래스 설정. 
	InBoss->AIControllerClass = AGTWBoss2PAIController::StaticClass();
	InBoss->AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 1페이즈 컨트롤러 해제.
	if (AController* ExistingController = InBoss->GetController())
	{
		ExistingController->UnPossess();
		ExistingController->Destroy();
	}

	// 2페이즈 AI컨트롤러 생성. 
	InBoss->SpawnDefaultController();


	// @TODO: 여기서는 맵에 존재하는 제어 대상들을 추가로 제어해야 하나?
}

AGTWBossPhase2GameMode::AGTWBossPhase2GameMode()
{
	// 디폴트 캐릭터 설정.
	static ConstructorHelpers::FClassFinder<AGTWPlayerCharacter> Player(
		TEXT("/Game/GTW/Cheracters/Player/BP_6FPlayer.BP_6FPlayer_C")
	);
	if (Player.Succeeded())
	{
		DefaultPawnClass = Player.Class;
	}

	// 플레이어 컨트롤러 클래스 설정.
	static ConstructorHelpers::FClassFinder<AController> PlayerController(
		TEXT("/Game/GTW/Cheracters/Player/BP_GTWPlayerContorller.BP_GTWPlayerContorller_C")
	);
	if (PlayerController.Succeeded())
	{
		PlayerControllerClass = PlayerController.Class;
	}
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
		Debug::Print(TEXT("[StartGameplay] World가 BeginPlay를 하지 않았음!"));
		return;
	}

	if (!IsValid(Boss.Get()))
	{
		Debug::Print(TEXT("[StartGameplay] 보스가 제대로 세팅되지 않았음"));
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
		Debug::Print(TEXT("[Phase2] GameInstance를 찾을 수 없음"));
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
		Debug::Print(TEXT("[Phase2] GameInstance가 없음"));
	}

	K2_OnPhaseFinished(CurrentState);
}

void AGTWBossPhase2GameMode::StopGameplay()
{
	K2_OnGameplayStopped();
}
