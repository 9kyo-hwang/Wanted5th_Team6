// Fill out your copyright notice in the Description page of Project Settings.


#include <GTWGameInstance.h>

#include <SaveGames/GTWClearHistorySaveGame.h>
#include <SaveGames/GTWProgressLevelSaveGame.h>
#include <UI/GTWTimerWidget.h>
#include <GTWDebugHelper.h>

#include <Kismet/GameplayStatics.h>

// 이거 개노가다임 내일 key값 저장해서 들어있는거에 대해서 반복문돌려서 DoesSaveGameExist 체크해서
// 코드 줄이기 안줄이면 복사 코드로 개많이 줄수만 늘어남
namespace
{
    //const FString ClearHistorySlotName = TEXT("ClearHistory");
    const FString ProgressLevelSlotName = TEXT("ProgressLevel");
    const FString TotalPlayTimeSlotName = TEXT("TotalPlayTime");
    const FString ClearSecondLevelSlotName = TEXT("SecondFloorClear");
    const FString ClearSokobanSlotName = TEXT("SokobanClear");
    const FString ClearShootingSlotName = TEXT("ShootingGameClear");
    const FString ClearBossPhase1LevelSlotName = TEXT("BossFirstPhaseGameClear");
    const FString ClearBossPhase2LevelSlotName = TEXT("BossSecondPhaseGameClear");
    constexpr int32 SaveGameInfoUserIndex = 0;
}

// 엔진 켤 때 한 번 호출됨
/*
* SaveGame으로부터 클리어 기록 로드
*   - 만약 SaveGame 없으면 생성 후 캐싱 
*/
void UGTWGameInstance::Init()
{
    Super::Init();

    InitClearHistory();
}

// 게임플레이 새로할 때마다 호출(값 초기화같은...)
void UGTWGameInstance::StartNewGame()
{
    SokobanClearTime = -1;
    ShootingClearTime = -1;
    bIsBossPhase1Cleared = false;
    TotalPlayTime = 0.0;
    CurrentFloorTime = 0.0;
    LevelClearTimes.Empty();
    bIsTickable = true;
    OnTimeChanged.Broadcast(TotalPlayTime);
}

void UGTWGameInstance::SaveProgressGame()
{
    const FString CurrentLevelName =
        UGameplayStatics::GetCurrentLevelName(this, true);

    if (CurrentLevelName.IsEmpty())
    {
        return;
    }

    if (UGameplayStatics::DoesSaveGameExist(ProgressLevelSlotName, SaveGameInfoUserIndex))
    {
        if (UGTWProgressLevelSaveGame* ProgressSaveGameInstance = 
            Cast<UGTWProgressLevelSaveGame>(UGameplayStatics::LoadGameFromSlot(
                ProgressLevelSlotName, SaveGameInfoUserIndex)))
        {
            ProgressSaveGame = ProgressSaveGameInstance;
            ProgressSaveGame->ProgressLevel = CurrentLevelName;
        }
        else
        {
            Debug::Print(TEXT("ProgressSaveGameInstance Load 실패"));
            return;
        }
    }
    else
    {
        if (UGTWProgressLevelSaveGame* ProgressSaveGameInstance = 
            Cast<UGTWProgressLevelSaveGame>(UGameplayStatics::CreateSaveGameObject(
                UGTWProgressLevelSaveGame::StaticClass())))
        {
            ProgressSaveGame = ProgressSaveGameInstance;
            ProgressSaveGame->ProgressLevel = CurrentLevelName;
        }
        else
        {
            Debug::Print(TEXT("ProgressSaveGameInstance 객체 생성 실패"));
            return;
        }
    }

    if (!IsValid(ProgressSaveGame))
    {
        Debug::Print(TEXT("ProgressSaveGame 객체가 유효하지 않음"));
        return;
    }

    Debug::Print(FString::Printf(
        TEXT("저장할 레벨 : %s"),
        *ProgressSaveGame->ProgressLevel));

    ProgressSaveGame->ProgressPlayTime = TotalPlayTime;

    if (!UGameplayStatics::SaveGameToSlot(
        ProgressSaveGame,
        ProgressLevelSlotName,
        SaveGameInfoUserIndex))
    {
        Debug::Print(TEXT("ProgressSaveGame 저장 실패"));
    }
    else
    {
        Debug::Print(TEXT("ProgressSaveGame 저장 성공"));
    }
}

bool UGTWGameInstance::StartProgressGame()
{
    if (UGameplayStatics::DoesSaveGameExist(ProgressLevelSlotName, SaveGameInfoUserIndex))
    {
        if (UGTWProgressLevelSaveGame* ProgressSaveGameInstance =
            Cast<UGTWProgressLevelSaveGame>(UGameplayStatics::LoadGameFromSlot(ProgressLevelSlotName,
                SaveGameInfoUserIndex)))
        {
            if (ProgressSaveGameInstance->ProgressLevel.IsEmpty())
            {
                return false;
            }

            // Resume the saved run without resetting its clear state.
            ProgressSaveGame = ProgressSaveGameInstance;
            TotalPlayTime = FMath::Max(0.0, ProgressSaveGameInstance->ProgressPlayTime);
            bIsTickable = true;
            OnTimeChanged.Broadcast(TotalPlayTime);
            OpenGameLevel(ProgressSaveGameInstance->ProgressLevel);

            return true;
        }
    }

    return false;
}

void UGTWGameInstance::CompleteGame()
{
    bIsTickable = false;

    if (!IsValid(ClearHistorySaveGame.Get()))
    {
        Debug::Print(TEXT("[GameInstance::CompleteGame] 클리어 시간은 확정됐으나 SaveGame 객체가 없어 저장 불가"));
        return;
    }

    //ClearHistorySaveGame->ClearTimes.Add(CurrentFloorTime);
    if (!SaveClearHistory())
    {
        Debug::Print(TEXT("[GameInstance::CompleteGame] 클리어 기록 저장 실패"));
    }
}

void UGTWGameInstance::Tick(float DeltaTime)
{
    if (!IsTickable())
    {
        return;
    }

    TotalPlayTime += DeltaTime;
    CurrentFloorTime += DeltaTime;
    OnTimeChanged.Broadcast(TotalPlayTime);
}

bool UGTWGameInstance::IsTickable() const
{
    // CDO(Class Default Object)가 아니고, 활성화 상태일 때만 틱이 돌도록 합니다.
    return bIsTickable && !IsTemplate() && GetWorld() && GetWorld()->IsGameWorld();
}

TStatId UGTWGameInstance::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UGTWGameInstance, STATGROUP_Tickables);
}

void UGTWGameInstance::RecordLevelClear(ELevelClear Level)
{
    if (LevelClearTimes.Contains(Level))
    {
        return;
    }
    // 여기다가 Switch 만들어서 층별로 저장

    LevelClearTimes.Add(Level, CurrentFloorTime);
    OnClearLevelChanged.Broadcast(Level, CurrentFloorTime);
    CurrentFloorTime = 0.0;
}

void UGTWGameInstance::RecordMiniGameClear(ELevelClear Level, double ClearTime)
{
    if (LevelClearTimes.Contains(Level))
    {
        return;
    }
    // 여기다가 Switch 만들어서 층별로 저장
    // 여긴 미니게임 저장
    LevelClearTimes.Add(Level, CurrentFloorTime);
    OnClearLevelChanged.Broadcast(Level, ClearTime);
}

void UGTWGameInstance::RecordProgressGameTime()
{
    if (UGameplayStatics::DoesSaveGameExist(TotalPlayTimeSlotName, SaveGameInfoUserIndex))
    {
        if (UGTWProgressLevelSaveGame* ProgressSaveGameInstance =
            Cast<UGTWProgressLevelSaveGame>(UGameplayStatics::LoadGameFromSlot(
                TotalPlayTimeSlotName, SaveGameInfoUserIndex)))
        {
            ProgressSaveGameInstance->ProgressPlayTime = TotalPlayTime;
            Debug::Print(TEXT("ProgressGameTime 성공"));
        }
        else
        {
            Debug::Print(TEXT("ProgressGameTime 실패"));
            return;
        }
    }
    else
    {
        if (UGTWProgressLevelSaveGame* ProgressSaveGameInstance =
            Cast<UGTWProgressLevelSaveGame>(UGameplayStatics::CreateSaveGameObject(
                UGTWProgressLevelSaveGame::StaticClass())))
        {
            ProgressSaveGameInstance->ProgressPlayTime = TotalPlayTime;
            Debug::Print(TEXT("ProgressGameTime 성공"));
        }
        else
        {
            Debug::Print(TEXT("ProgressGameTime 실패"));
            return;
        }
    }

    if (UGameplayStatics::SaveGameToSlot(
        ProgressSaveGame,
        TotalPlayTimeSlotName,
        SaveGameInfoUserIndex) == false)
    {
        Debug::Print(TEXT("ProgressSaveGame 저장 실패"));
    }
    else
    {
        Debug::Print(TEXT("ProgressSaveGame 저장 성공"));
    }
}

void UGTWGameInstance::InitializeTimerWidget(UGTWTimerWidget* Widget) const
{
    if (!IsValid(Widget))
    {
        return;
    }
    Widget->UpdateTimer(TotalPlayTime);
    for (const auto& Entry : LevelClearTimes)
    {
        Widget->UpdateClearLevel(Entry.Key, Entry.Value);
    }
}

bool UGTWGameInstance::OpenGameLevel(FString LevelName)
{
    if (LevelName.IsEmpty())
        return false;

   const FString LevelPath = FPaths::Combine(FPaths::ProjectContentDir(),
        TEXT("GTW/Maps/"), LevelName);

    // 레벨 패키지가 실제로 존재하는지 확인
    if (FPackageName::DoesPackageExist(LevelPath) == false)
    {
        return false;
    }

    if (LevelName == TEXT("Title"))
    {
        RecordProgressGameTime();
    }

    UGameplayStatics::OpenLevel(
        this,
        FName(LevelName));

    return true;
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
        RecordMiniGameClear(ELevelClear::SokobanClear, SokobanClearTime);
        return true;
    case EMinigameType::Shooting:
        ShootingClearTime = InClearTimeSeconds;
        RecordMiniGameClear(ELevelClear::ShootingGameClear, ShootingClearTime);
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
    if (TotalPlayTime > 0.0)
    {
        return TotalPlayTime;
    }

    return 0.0;
}

void UGTWGameInstance::InitClearHistory()
{
    if (UGameplayStatics::DoesSaveGameExist(ClearHistorySlotName, SaveGameInfoUserIndex))
    {
        if (UGTWClearHistorySaveGame* ClearHistorySaveGameInstance = Cast<UGTWClearHistorySaveGame>(UGameplayStatics::LoadGameFromSlot(ClearHistorySlotName, SaveGameInfoUserIndex)))
        {
            ClearHistorySaveGame = ClearHistorySaveGameInstance;
        }
        else
        {
            Debug::Print(TEXT("[GameInstance::Init] 기존 클리어 로드 실패"));
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
            Debug::Print(TEXT("[GameInstance::Init] SaveGame 객체 생성 실패"));
            return;
        }
    }
}

// return: 저장 성공 시 true, 실패했거나 SaveGame이 Invalid 하면 false
bool UGTWGameInstance::SaveClearHistory()
{
    if (!IsValid(ClearHistorySaveGame.Get()))
    {
        return false;
    }

    // SlotName이랑 UserIndex는 확인 필요
    return UGameplayStatics::SaveGameToSlot(ClearHistorySaveGame, ClearHistorySlotName, SaveGameInfoUserIndex);
}
