// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/GTWBaseBossGameMode.h"

#include <GTWDebugHelper.h>
#include <Interfaces/GTWBossAIInterface.h>
#include <Controllers/GTWPlayerController.h>
#include <Characters/Player/GTWPlayerCharacter.h>
#include <Kismet/GameplayStatics.h>

#include <Engine/World.h>

AGTWBaseBossGameMode::AGTWBaseBossGameMode()
{
	// @TODO: BP 컨트롤러로 교체?
	PlayerControllerClass = AGTWPlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<AGTWPlayerCharacter> PlayerCharacterRef(
		TEXT("/Game/GTW/Characters/Player/BP_GTWPlayerCharacter.BP_GTWPlayerCharacter_C")
	);

	if (PlayerCharacterRef.Succeeded())
	{
		DefaultPawnClass = PlayerCharacterRef.Class;
	}
}

// Level Blueprint에서 배치한 보스를 넘겨줄 예정
void AGTWBaseBossGameMode::InitPhase(AActor* InBoss)
{
	if (CurrentState != EBossGameModeState::Ready)
	{
		return;
	}

	if (!IsValid(InBoss) || !Cast<IGTWBossAIInterface>(InBoss) || IsValid(Boss.Get()))
	{
		Debug::Print(TEXT("[BaseBossGameMode::InitPhase] 보스 세팅에 문제가 있음"));
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
		!GetWorld() || !GetWorld()->HasBegunPlay())
	{
		Debug::Print(TEXT("[BaseBossGameMode::BeginPhase] 페이즈 시작 전에 유효하지 않은 세팅"));
		return;
	}
	
	if (!IsValid(Boss.Get()))
	{
		Debug::Print(TEXT("[BaseBossGameMode::BeginPhase] 보스 가동해야 하는데 유효하지 않음"));
		return;
	}

	IGTWBossAIInterface* BossAIInterface = Cast<IGTWBossAIInterface>(Boss);
	if (!BossAIInterface)
	{
		Debug::Print(TEXT("[BaseBossGameMode::BeginPhase] 보스 가동해야 하는데 유효하지 않음"));
		return;
	}

	CurrentState = EBossGameModeState::Play;

	BossAIInterface->StartCombat();
	if (IsPhasePlaying())
	{
		SetIgnorePlayerInput(false);
		K2_OnPhaseStarted();
	}
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

	if (!RequestPhaseOutro(Result))
	{
		Debug::Print(TEXT("[BaseBossGameMode::EndPhase] 아웃트로가 없어서 즉시 완료 처리."));
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
		if (IGTWBossAIInterface* BossAIInterface = Cast<IGTWBossAIInterface>(Boss))
		{
			BossAIInterface->StopCombat();
		}
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

