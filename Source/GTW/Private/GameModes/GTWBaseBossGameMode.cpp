// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/GTWBaseBossGameMode.h"

#include <GTWDebugHelper.h>
#include <Interfaces/GTWBossAIInterface.h>
#include <Controllers/GTWPlayerController.h>
#include <Characters/Player/GTWPlayerCharacter.h>
#include <GTWGameInstance.h>

#include <Kismet/GameplayStatics.h>
#include <Engine/World.h>

AGTWBaseBossGameMode::AGTWBaseBossGameMode()
{
	PlayerControllerClass = AGTWPlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<AGTWPlayerCharacter> PlayerCharacterRef(
		TEXT("/Game/GTW/Characters/Player/BP_GTWPlayerCharacter.BP_GTWPlayerCharacter_C")
	);

	if (PlayerCharacterRef.Succeeded())
	{
		DefaultPawnClass = PlayerCharacterRef.Class;
	}
}

bool AGTWBaseBossGameMode::SetupBoss()
{
	UClass* BossClass = GetBossClass();
	if (!BossClass)
	{
		Debug::Print(TEXT("[Base GM:SetupBoss] 보스 클래스 가져오기 실패"));
		return false;
	}

	TArray<AActor*> BossActors;
	UGameplayStatics::GetAllActorsOfClass(this, BossClass, BossActors);
	if (BossActors.Num() != 1 || !IsValid(BossActors[0]) || !Cast<IGTWBossAIInterface>(BossActors[0]) || IsValid(Boss.Get()))
	{
		Debug::Print(TEXT("[Base GM:SetupBoss] 맵에 GTWBoss 인스턴스 1개가 있는 게 아닌 것 같음"));
		return false;
	}

	Boss = BossActors[0];
	if (!BindBossEvents())
	{
		UnbindBossEvents();
		Boss = nullptr;

		Debug::Print(TEXT("[Base GM:SetupBoss] 보스 타입 또는 델리게이트 바인딩이 유효하지 않음"));
		return false;
	}

	return true;
}

void AGTWBaseBossGameMode::StartPlay()
{
	Super::StartPlay();

	SetIgnorePlayerInput(true);

	if (!SetupBoss())
	{
		SetIgnorePlayerInput(false);
		return;
	}

	// 해당 레벨에서 [이어하기]를 할 수 있도록
	UGTWGameInstance* GI = Cast<UGTWGameInstance>(GetGameInstance());
	if (!GI)
	{
		Debug::Print(TEXT("[Base GM::StartPlay] GTWGameInstance 획득 실패"));
		return;
	}

	GI->SaveProgressGame();

	NativeBeginSetup_BeforeCombat();
}

void AGTWBaseBossGameMode::NativeBeginSetup_BeforeCombat()
{
	if (CurrentState != EBossGameModeState::Ready ||
		bWaitingForBeforeCombatSetup)
	{
		return;
	}

	bWaitingForBeforeCombatSetup = true;

	BP_BeginSetup_BeforeCombat();
}

void AGTWBaseBossGameMode::CompleteSetup_BeforeCombat()
{
	if (!bWaitingForBeforeCombatSetup || 
		CurrentState != EBossGameModeState::Ready)
	{
		return;
	}

	bWaitingForBeforeCombatSetup = false;

	BeginCombat();
}

void AGTWBaseBossGameMode::BeginCombat()
{
	if (CurrentState != EBossGameModeState::Ready || 
		!GetWorld() || !GetWorld()->HasBegunPlay())
	{
		Debug::Print(TEXT("[Base GM::BeginCombat] 페이즈 시작 전에 유효하지 않은 세팅"));
		return;
	}
	
	if (!IsValid(Boss.Get()))
	{
		Debug::Print(TEXT("[Base GM::BeginCombat] 보스 가동해야 하는데 유효하지 않음"));
		return;
	}

	IGTWBossAIInterface* BossAIInterface = Cast<IGTWBossAIInterface>(Boss);
	if (!BossAIInterface)
	{
		Debug::Print(TEXT("[Base GM::BeginCombat] 보스 가동해야 하는데 유효하지 않음"));
		return;
	}

	CurrentState = EBossGameModeState::Play;

	BossAIInterface->StartCombat();
	if (IsPhasePlaying())
	{
		SetIgnorePlayerInput(false);
	}
}

void AGTWBaseBossGameMode::Clear()
{
	EndCombat(EBossGameModeState::Clear);
}

// Clear/Fail 공통으로 처리해야 할 로직: 입력 Block, 보스 Stop, 하위 GM 별 추가 처리 및 아웃트로
void AGTWBaseBossGameMode::EndCombat(EBossGameModeState Result)
{
	if (!IsPhasePlaying())
	{
		Debug::Print(TEXT("[Base GM::EndCombat] 페이즈 끝나는 시점에 Play 상태가 아님"));
		return;
	}

	if (Result != EBossGameModeState::Clear &&
		Result != EBossGameModeState::Fail)
	{
		Debug::Print(TEXT("[BaseBossGameMode::EndCombat] Clear 또는 Fail만 가능"));
		return;
	}

	CurrentState = Result;

	SetIgnorePlayerInput(true);
	StopBoss();

	RecordCombatResult(Result);

	NativeBeginSetup_AfterCombat();
}

void AGTWBaseBossGameMode::NativeBeginSetup_AfterCombat()
{
	if (CurrentState != EBossGameModeState::Clear &&
		CurrentState != EBossGameModeState::Fail)
	{
		return;
	}

	if (bWaitingForAfterCombatSetup)
	{
		return;
	}

	bWaitingForAfterCombatSetup = true;
	BP_BeginSetup_AfterCombat(CurrentState);
}

void AGTWBaseBossGameMode::CompleteSetup_AfterCombat()
{
	if (!bWaitingForAfterCombatSetup)
	{
		return;
	}

	if (CurrentState != EBossGameModeState::Clear &&
		CurrentState != EBossGameModeState::Fail)
	{
		return;
	}

	bWaitingForAfterCombatSetup = false;
	OpenNextLevelByResult(CurrentState);
}

void AGTWBaseBossGameMode::SetIgnorePlayerInput(bool bNewInput)
{
	AGTWPlayerController* GTWPC = Cast<AGTWPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
	if (!GTWPC)
	{
		Debug::Print(TEXT("[Base GM::SetIgnorePlayerInput] GTW 컨트롤러 캐스팅 실패"));
		return;
	}

	if (bPlayerInputIgnored == bNewInput)
	{
		return;
	}

	bPlayerInputIgnored = bNewInput;

	GTWPC->SetIgnoreMoveInput(bNewInput);
	GTWPC->SetIgnoreLookInput(bNewInput);
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

void AGTWBaseBossGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	bWaitingForBeforeCombatSetup = false;
	bWaitingForAfterCombatSetup = false;

	StopBoss();

	Super::EndPlay(EndPlayReason);
}

bool AGTWBaseBossGameMode::OpenNextLevelByResult(EBossGameModeState Result)
{
	const TSoftObjectPtr<UWorld>& LevelToOpen
		= Result == EBossGameModeState::Clear
		? ClearLevel
		: FailLevel;

	if (LevelToOpen.IsNull())
	{
		Debug::Print(TEXT("[Base GM::OpenNextLevelByResult] 이동할 레벨이 지정되지 않음"));
		return false;
	}

	// 화면 전환 시 현재 레벨의 모습을 보이지 않게 하기 위해 까맣게 처리. 
	APlayerCameraManager* manager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (IsValid(manager))
	{
		manager->SetManualCameraFade(
			1.0f,				// 완~저니 검정
			FLinearColor::Black,// 페이드 색
			false				// 페이드 오디오
		);
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelToOpen);
	return true;
}
