// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/GTWGameInstance.h"

#include <SaveGames/GTWClearHistorySaveGame.h>

#include <Kismet/GameplayStatics.h>

namespace
{
    const FString ClearHistorySlotName = TEXT("ClearHistory");
    constexpr int32 ClearHistoryUserIndex = 0;

    static void Print(const FString& Message, FColor Color = FColor::MakeRandomColor(), int32 InKey = -1)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(InKey, 7.f, Color, Message);
            UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
        }
    }
}

// 엔진 켤 때 한 번 호출됨
/*
* SaveGame으로부터 클리어 기록 로드
*   - 만약 SaveGame 없으면 생성 후 캐싱 
*/
void UGTWGameInstance::Init()
{
    Super::Init();

    if (UGameplayStatics::DoesSaveGameExist(ClearHistorySlotName, ClearHistoryUserIndex))
    {
        if (UGTWClearHistorySaveGame* ClearHistorySaveGameInstance = Cast<UGTWClearHistorySaveGame>(UGameplayStatics::LoadGameFromSlot(ClearHistorySlotName, ClearHistoryUserIndex)))
        {
            ClearHistorySaveGame = ClearHistorySaveGameInstance;
        }
        else
        {
            Print(TEXT("[GameInstance::Init] 기존 클리어 로드 실패"));
            return;
        }
    }
    else
    {
        if (UGTWClearHistorySaveGame* ClearHistorySaveGameInstance = Cast<UGTWClearHistorySaveGame>(UGameplayStatics::CreateSaveGameObject(UGTWClearHistorySaveGame::StaticClass())))
        {
            ClearHistorySaveGame = ClearHistorySaveGameInstance;
        }
        else
        {
            Print(TEXT("[GameInstance::Init] SaveGame 객체 생성 실패"));
            return;
        }
    }
}

// 게임플레이 새로할 때마다 호출(값 초기화같은...)
void UGTWGameInstance::StartNewGame()
{
    SokobanClearTime = -1;
    ShootingClearTime = -1;
    bIsBossPhase1Cleared = false;
    GameStartTimeSeconds = FPlatformTime::Seconds();
    GameClearTimeSeconds = -1;
}

void UGTWGameInstance::CompleteGame()
{
    if (GameStartTimeSeconds < 0.0)
    {
        Print(TEXT("[GameInstance::CompleteGame] 아직 게임을 시작하지 않았음"));
        return;
    }

    if (GameClearTimeSeconds >= 0.0)
    {
        Print(TEXT("[GameInstance::CompleteGame] 이미 이번 게임에 대해서는 처리가 완료됨"));
        return;
    }

    GameClearTimeSeconds = FPlatformTime::Seconds() - GameStartTimeSeconds;

    if (!IsValid(ClearHistorySaveGame.Get()))
    {
        Print(TEXT("[GameInstance::CompleteGame] 클리어 시간은 확정됐으나 SaveGame 객체가 없어 저장 불가"));
        return;
    }

    ClearHistorySaveGame->ClearTimes.Add(GameClearTimeSeconds);
    if (!SaveClearHistory())
    {
        Print(TEXT("[GameInstance::CompleteGame] 클리어 기록 저장 실패"));
    }
}

bool UGTWGameInstance::SetMinigameClearTime(EMinigameType Type, double InClearTimeSeconds)
{
    if (InClearTimeSeconds < 0.0 || !FMath::IsFinite(InClearTimeSeconds))
    {
        return false;
    }

    switch (Type)
    {
    case EMinigameType::Sokoban:
        SokobanClearTime = InClearTimeSeconds;
        return true;
    case EMinigameType::Shooting:
        ShootingClearTime = InClearTimeSeconds;
        return true;
    default:
        return false;
    }
}

bool UGTWGameInstance::TryGetMinigameClearTime(EMinigameType Type, double& OutClearTimeSeconds) const 
{
    double Retval = -1;
    switch (Type)
    {
    case EMinigameType::Sokoban:
        Retval = SokobanClearTime;
        break;
    case EMinigameType::Shooting:
        Retval = ShootingClearTime;
        break;
    default:
        break;
    }

    if (Retval < 0.0)
    {
        return false;
    }
    
    OutClearTimeSeconds = Retval;
    return true;
}

double UGTWGameInstance::GetPlaytimeSeconds() const
{
    if (GameStartTimeSeconds < 0.0)
    {
        return 0.0;
    }

    if (GameClearTimeSeconds >= 0.0)
    {
        return GameClearTimeSeconds;
    }

    return FPlatformTime::Seconds() - GameStartTimeSeconds;
}

int32 UGTWGameInstance::NumClearTimes() const
{
    if (IsValid(ClearHistorySaveGame.Get()))
    {
        return ClearHistorySaveGame->ClearTimes.Num();
    }

    Print(TEXT("ClearHistorySaveGame 객체가 없네?"));
    return 0;
}

// return: 저장 성공 시 true, 실패했거나 SaveGame이 Invalid 하면 false
bool UGTWGameInstance::SaveClearHistory()
{
    if (!IsValid(ClearHistorySaveGame.Get()))
    {
        return false;
    }

    // SlotName이랑 UserIndex는 확인 필요
    return UGameplayStatics::SaveGameToSlot(ClearHistorySaveGame, ClearHistorySlotName, ClearHistoryUserIndex);
}
