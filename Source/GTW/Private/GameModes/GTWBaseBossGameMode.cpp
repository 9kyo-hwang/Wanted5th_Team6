// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/GTWBaseBossGameMode.h"

#include <GTWDebugHelper.h>
#include <Characters/Boss/GTWBoss.h>
#include <Controllers/GTWPlayerController.h>
#include <Kismet/GameplayStatics.h>

#include <Engine/World.h>

AGTWBaseBossGameMode::AGTWBaseBossGameMode()
{
	// @TODO: BP 컨트롤러로 교체?
	PlayerControllerClass = AGTWPlayerController::StaticClass();
}

// Level Blueprint에서 배치한 보스를 넘겨줄 예정
void AGTWBaseBossGameMode::InitPhase(AGTWBoss* InBoss)
{
	if (CurrentState != EBossGameModeState::Ready || !IsValid(InBoss))
	{
		Debug::Print(TEXT("[BaseBossGameMode::InitPhase] 세팅할 보스가 유효하지 않음"));
		return;
	}

	// 레벨에 배치된 보스를 시작 전에 연결해야 하므로, 아직 유효하면 안됨
	if (IsValid(Boss.Get()))
	{
		Debug::Print(TEXT("[BaseBossGameMode::InitPhase] 아직 보스 세팅 안했는데 있네?"));
		return;
	}

	Boss = InBoss;

	if (!BindBossEvents())
	{
		UnbindBossEvents();
		Boss = nullptr;

		Debug::Print(TEXT("[BaseBossGameMode::InitPhase] 보스 타입 또는 델리게이트 바인딩이 유효하지 않음"));
	}
}

void AGTWBaseBossGameMode::StartPlay()
{
	Super::StartPlay();

	SetIgnorePlayerInput(true);

	if (!IsValid(Boss.Get()))
	{
		Debug::Print(TEXT("[BaseBossGameMode::StartPlay] 실행해야 할 보스가 유효하지 않음"));
		return;
	}

	if (OnIntroRequested.IsBound())
	{
		Debug::Print(TEXT("[BaseBossGameMode::StartPlay] 인트로 재생! 종료 시 BeginPhase"));
		OnIntroRequested.Broadcast();
	}
	else
	{
		BeginPhase();
	}
}

void AGTWBaseBossGameMode::BeginPhase()
{
	if (CurrentState != EBossGameModeState::Ready || 
		!GetWorld() || !GetWorld()->HasBegunPlay() || 
		!IsValid(Boss.Get()))
	{
		Debug::Print(TEXT("[BaseBossGameMode::BeginPhase] 페이즈 시작 전에 유효하지 않은 세팅"));
		return;
	}

	CurrentState = EBossGameModeState::Play;

	SetIgnorePlayerInput(false);

	// @TODO: 보스 베이스 타입 추가되면
	// Boss->StartCombat();

	if (IsPhasePlaying())
	{
		K2_OnPhaseStarted();
	}
}

void AGTWBaseBossGameMode::FailPhase()
{
	EndPhase(EBossGameModeState::Fail);
}

void AGTWBaseBossGameMode::ClearPhase()
{
	EndPhase(EBossGameModeState::Clear);
}

// Clear/Fail 공통으로 처리해야 할 로직: 입력 Block, 보스 Stop, 하위 GM 별 추가 처리 및 아웃트로
void AGTWBaseBossGameMode::EndPhase(EBossGameModeState Result)
{
	if (CurrentState != EBossGameModeState::Play)
	{
		Debug::Print(TEXT("[BaseBossGameMode::EndPhase] 페이즈 끝나는 시점에 Play 상태가 아님"));
		return;
	}

	if (Result != EBossGameModeState::Clear &&
		Result != EBossGameModeState::Fail)
	{
		Debug::Print(TEXT("[BaseBossGameMode::EndPhase] Clear 또는 Fail만 가능"));
		return;
	}

	CurrentState = Result;
	SetIgnorePlayerInput(true);
	StopBoss();

	K2_OnPhaseEnded();
	HandlePhaseResult(Result);

	if (OnOutroRequested.IsBound())
	{
		Debug::Print(TEXT("[BaseBossGameMode::EndPhase] 아웃트로 재생. 완료 후 레벨 변경 처리"));
		OnOutroRequested.Broadcast(CurrentState);
	}
	else
	{
		CompletePhase();
	}
}

void AGTWBaseBossGameMode::SetIgnorePlayerInput(bool bNewInput)
{
	AGTWPlayerController* PlayerController = Cast<AGTWPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (!PlayerController)
	{
		Debug::Print(TEXT("[Base GM::SetIgnorePlayerInput] 컨트롤러가 GTW 것이 아님?"));
		return;
	}

	// @TODO: Lock/Unlock 짝이 맞아야 해서, 나중에 PC에 별도 API를 하나 둬야할 듯
	PlayerController->SetIgnoreMoveInput(bNewInput);
	PlayerController->SetIgnoreLookInput(bNewInput);
}

void AGTWBaseBossGameMode::StopBoss()
{
	UnbindBossEvents();

	if (IsValid(Boss.Get()))
	{
		//Boss->StopCombat();
	}
}

void AGTWBaseBossGameMode::CompletePhase()
{
	if (bPhaseCompletionRequested)
	{
		return;
	}

	if (CurrentState != EBossGameModeState::Clear &&
		CurrentState != EBossGameModeState::Fail)
	{
		return;
	}

	bPhaseCompletionRequested = true;
	if (!HandlePhaseCompletion(CurrentState))
	{
		// 자식 GM에서 페이즈 정리가 제대로 되지 않으면 return false
		bPhaseCompletionRequested = false;
	}
}

void AGTWBaseBossGameMode::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	StopBoss();

	Super::EndPlay(EndPlayReason);
}

