// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/GTWBossPhase2GameMode.h"

#include <GTWGameInstance.h>
#include <GTWDebugHelper.h>
#include <Characters/Player/GTWPlayerCharacter.h>
#include <Characters/Boss/GTWBossPhase2Character.h>
#include <Controllers/GTWBoss2PAIController.h>
#include <Controllers/GTWPlayerController.h>

#include <Kismet/GameplayStatics.h>

AGTWBossPhase2GameMode::AGTWBossPhase2GameMode()
{
	// 2페용 캐릭터 재설정
	static ConstructorHelpers::FClassFinder<AGTWPlayerCharacter> PlayerCharacterRef(
		TEXT("/Game/GTW/Characters/Player/BP_6FPlayer.BP_6FPlayer_C")
	);

	if (PlayerCharacterRef.Succeeded())
	{
		DefaultPawnClass = PlayerCharacterRef.Class;
	}
}

void AGTWBossPhase2GameMode::OnGoalReached()
{
	if (!IsPhasePlaying())
	{
		return;
	}

	ClearPhase();
}

void AGTWBossPhase2GameMode::FailPhase(EBossPhase2FailReason InFailReason)
{
	if (!IsPhasePlaying())
	{
		return;
	}

	FailReason = InFailReason;
	EndPhase(EBossGameModeState::Fail);
}

bool AGTWBossPhase2GameMode::BindBossEvents()
{
	return true;
}

void AGTWBossPhase2GameMode::UnbindBossEvents()
{

}

void AGTWBossPhase2GameMode::HandlePhaseResult(EBossGameModeState Result)
{
	UGTWGameInstance* GameInstance = GetGameInstance<UGTWGameInstance>();
	if (!GameInstance)
	{
		Debug::Print(TEXT("[P2 GM::HandlePhaseResult] GameInstance가 없음"));
		return;
	}

	if (Result == EBossGameModeState::Clear)
	{
		// 클리어 기록만 남기자
		GameInstance->CompleteGame();
	}
	else
	{
		GameInstance->ResetBossPhase1Cleared();
	}
}

bool AGTWBossPhase2GameMode::RequestPhaseOutro(EBossGameModeState Result)
{
	if (OnOutroRequested.IsBound())
	{
		switch (Result)
		{
		case EBossGameModeState::Clear: 
		{
			OnOutroRequested.Broadcast(EBossPhase2OutroTypes::Cleared);
			return true;
		}
		
		case EBossGameModeState::Fail:
		{
			switch (FailReason)
			{
			case EBossPhase2FailReason::Fall:
				OnOutroRequested.Broadcast(EBossPhase2OutroTypes::Fall);
				return true;
			case EBossPhase2FailReason::BossContact:
				OnOutroRequested.Broadcast(EBossPhase2OutroTypes::BossContact);
				return true;
			default: 
				return false;
			}
		}

		default: 
			return false;
		}
	}

	return false;
}

bool AGTWBossPhase2GameMode::HandlePhaseCompletion(EBossGameModeState Result)
{
	const TSoftObjectPtr<UWorld>& LevelToOpen
		= Result == EBossGameModeState::Clear
		? ClearLevel
		: Phase1Level;

	if (LevelToOpen.IsNull())
	{
		Debug::Print(TEXT("[P2 GM::HandlePhaseCompletion] 이동할 레벨이 지정되지 않음"));
		return false;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelToOpen);
	return true;
}
