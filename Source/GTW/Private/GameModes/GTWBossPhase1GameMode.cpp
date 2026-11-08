// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/GTWBossPhase1GameMode.h"

#include <GTWGameInstance.h>
#include <GTWDebugHelper.h>
#include <Characters/Boss/GTWBoss1.h>
#include <Kismet/GameplayStatics.h>

/*
* [x] 보스 참조 저장
* [x] 보스 명중 델리게이트 구독
* [ ] 연출 세팅은 Level BP에서??
*/

AGTWBossPhase1GameMode::AGTWBossPhase1GameMode()
{

}

void AGTWBossPhase1GameMode::FailPhase()
{
	EndPhase(EBossGameModeState::Fail);
}

bool AGTWBossPhase1GameMode::BindBossEvents()
{
	Super::BindBossEvents();

	AGTWBoss1* Phase1Boss = Cast<AGTWBoss1>(GetBoss());
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
	Super::UnbindBossEvents();
 
	if (BossHitDelegateHandle.IsValid())
	{
		if (AGTWBoss1* Phase1Boss = Cast<AGTWBoss1>(GetBoss()))
		{
			Phase1Boss->OnObjectHit.Remove(BossHitDelegateHandle);
		}
		else
		{
			Debug::Print(TEXT("[P1 GM : UnbindBossEvents] 1페이즈 보스 인스턴스가 아니네?"));
		}
	}

	BossHitDelegateHandle.Reset();
}

// 연출 실행 전 -> GI에게 클리어 유무 기록하기
void AGTWBossPhase1GameMode::HandlePhaseResult(EBossGameModeState Result)
{
	UGTWGameInstance* GameInstance = GetGameInstance<UGTWGameInstance>();
	if (!GameInstance)
	{
		Debug::Print(TEXT("[P1 GM::HandlePhaseResult] GameInstance가 없음"));
		return;
	}

	if (Result == EBossGameModeState::Clear)
	{
		GameInstance->SetBossPhase1Cleared();
	}
	else
	{
		GameInstance->ResetBossPhase1Cleared();
	}
}

bool AGTWBossPhase1GameMode::RequestPhaseOutro(EBossGameModeState Result)
{
	if (OnOutroRequested.IsBound())
	{
		switch (Result)
		{
		case EBossGameModeState::Clear:
			OnOutroRequested.Broadcast(EBossPhase1OutroTypes::Cleared);
			break;
		case EBossGameModeState::Fail:	// 얘는 떨어지는 것 밖에 없음
			OnOutroRequested.Broadcast(EBossPhase1OutroTypes::Fall);
			break;
		}

		return true;
	}

	return false;
}

// 연출 실행 후 -> 실제로 레벨 이동 처리
bool AGTWBossPhase1GameMode::HandlePhaseCompletion(EBossGameModeState Result)
{
	const TSoftObjectPtr<UWorld>& LevelToOpen 
		= Result == EBossGameModeState::Clear
		? Phase2Level 
		: Phase1Level;

	if (LevelToOpen.IsNull())
	{
		Debug::Print(TEXT("[P1 GM::HandlePhaseCompletion] 이동할 레벨이 지정되지 않음"));
		return false;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelToOpen);
	return true;
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
		ClearPhase();
	}
}
