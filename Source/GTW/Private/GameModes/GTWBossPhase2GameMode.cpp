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

void AGTWBossPhase2GameMode::Fail(EBossPhase2FailReason InFailReason)
{
	FailReason = InFailReason;
	EndCombat(EBossGameModeState::Fail);
}

UClass* AGTWBossPhase2GameMode::GetBossClass() const
{
	return AGTWBossPhase2Character::StaticClass();
}

void AGTWBossPhase2GameMode::RecordCombatResult(EBossGameModeState Result)
{
	UGTWGameInstance* GameInstance = GetGameInstance<UGTWGameInstance>();
	if (!GameInstance)
	{
		Debug::Print(TEXT("[P2 GM::RecordCombatResult] GameInstance가 없음"));
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
