// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/GTWBossPhase1GameMode.h"

#include <GTWGameInstance.h>
#include <GTWDebugHelper.h>
#include <Characters/Boss/GTWBossPhase1Character.h>
#include <Kismet/GameplayStatics.h>

AGTWBossPhase1GameMode::AGTWBossPhase1GameMode()
{
	FailLevel = FSoftObjectPath(TEXT("/Game/GTW/Maps/BossFirstPhase.BossFirstPhase"));
	ClearLevel = FSoftObjectPath(TEXT("/Game/GTW/Maps/BossSecondPhase.BossSecondPhase"));
}

void AGTWBossPhase1GameMode::Fail()
{
	EndCombat(EBossGameModeState::Fail);
}

UClass* AGTWBossPhase1GameMode::GetBossClass() const
{
	return AGTWBossPhase1Character::StaticClass();
}

bool AGTWBossPhase1GameMode::BindBossEvents()
{
	AGTWBossPhase1Character* Phase1Boss = Cast<AGTWBossPhase1Character>(GetBoss());
	if (!Phase1Boss)
	{
		Debug::Print(TEXT("[P1 GM : BindBossEvents] 1페이즈 보스 인스턴스가 아니네?"));
		return false;
	}

	// 1페 보스 캐릭터로 캐스팅, 실패 시 return false
	BossHitDelegateHandle = Phase1Boss->OnObjectHit.AddUObject(
		this,
		&ThisClass::OnHitBoss
	);

	CurrentHitCount = 0;

	return true;
}

void AGTWBossPhase1GameMode::UnbindBossEvents()
{ 
	if (!BossHitDelegateHandle.IsValid())
	{
		return;
	}

	if (AGTWBossPhase1Character* Phase1Boss = Cast<AGTWBossPhase1Character>(GetBoss()))
	{
		Phase1Boss->OnObjectHit.Remove(BossHitDelegateHandle);
	}

	BossHitDelegateHandle.Reset();
}

// 연출 실행 전 -> GI에게 클리어 유무 기록하기
void AGTWBossPhase1GameMode::RecordCombatResult(EBossGameModeState Result)
{
	UGTWGameInstance* GameInstance = GetGameInstance<UGTWGameInstance>();
	if (!GameInstance)
	{
		Debug::Print(TEXT("[P1 GM::RecordCombatResult] GameInstance가 없음"));
		return;
	}

	if (Result == EBossGameModeState::Clear)
	{
		GameInstance->RecordLevelClear(ELevelClear::BossFirstPhaseGameClear);
	}
}

void AGTWBossPhase1GameMode::OnHitBoss()
{
	if (!IsPhasePlaying())
	{
		return;
	}

	// 카운트 올렸을 때 횟수 충족했다면 Clear!
	if (++CurrentHitCount >= ClearHitCount)
	{
		Clear();
	}
}
