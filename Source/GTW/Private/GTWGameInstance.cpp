// Fill out your copyright notice in the Description page of Project Settings.


#include <GTWGameInstance.h>

#include <SaveGames/GTWClearHistorySaveGame.h>
#include <SaveGames/GTWProgressLevelSaveGame.h>
#include <UI/GTWTimerWidget.h>
#include <GTWDebugHelper.h>

#include <Kismet/GameplayStatics.h>

namespace
{
    const FString ProgressLevelSlotName = TEXT("ProgressLevel");
    const FString TotalPlayTimeSlotName = TEXT("TotalPlayTime");
    const FString ClearSecondLevelSlotName = TEXT("SecondFloorClear");
    const FString ClearSokobanSlotName = TEXT("SokobanClear");
    const FString ClearShootingSlotName = TEXT("ShootingGameClear");
    const FString ClearBossPhase1LevelSlotName = TEXT("BossFirstPhaseGameClear");
    const FString ClearBossPhase2LevelSlotName = TEXT("BossSecondPhaseGameClear");
    constexpr int32 SaveGameInfoUserIndex = 0;
}

UGTWGameInstance::UGTWGameInstance()
{
    CachingSaveDataKeyvalue = { ProgressLevelSlotName,TotalPlayTimeSlotName,
    ClearSecondLevelSlotName,ClearSokobanSlotName,ClearShootingSlotName,
    ClearBossPhase1LevelSlotName, ClearBossPhase2LevelSlotName };
}

// 엔진 켤 때 한 번 호출됨
/*
* SaveGame으로부터 클리어 기록 로드
*   - 만약 SaveGame 없으면 생성 후 캐싱 
*/
void UGTWGameInstance::Init()
{
    Super::Init();
}

// 게임플레이 새로할 때마다 호출(값 초기화같은...)
void UGTWGameInstance::StartNewGame()
{
    // 1. 기존 SaveGame 파일 삭제
    for (const FString& SlotName : CachingSaveDataKeyvalue)
    {
        if (!UGameplayStatics::DoesSaveGameExist(
            SlotName, SaveGameInfoUserIndex))
        {
            continue;
        }

        if (!UGameplayStatics::DeleteGameInSlot(
            SlotName, SaveGameInfoUserIndex))
        {
            Debug::Print(FString::Printf(
                TEXT("SaveGame 삭제 실패 : %s"),
                *SlotName));

            return;
        }
    }

    // 2. 메모리에 남은 SaveGame 객체 초기화
    ProgressSaveGame = nullptr;
    ClearHistorySaveGame = nullptr;

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

    ProgressSaveGame->ProgressTotalPlayTime = TotalPlayTime;
    ProgressSaveGame->ProgressCurrentFloorPlayTime = CurrentFloorTime;
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

bool UGTWGameInstance::LoadProgressGame()
{
    if (!UGameplayStatics::DoesSaveGameExist(
        ProgressLevelSlotName,
        SaveGameInfoUserIndex))
    {
        return false;
    }

    UGTWProgressLevelSaveGame* ProgressSaveGameInstance =
        Cast<UGTWProgressLevelSaveGame>(
            UGameplayStatics::LoadGameFromSlot(
                ProgressLevelSlotName,
                SaveGameInfoUserIndex));

    if (!IsValid(ProgressSaveGameInstance) ||
        ProgressSaveGameInstance->ProgressLevel.IsEmpty())
    {
        return false;
    }

    // 진행 정보 복원
    ProgressSaveGame = ProgressSaveGameInstance;

    TotalPlayTime = FMath::Max(0.0, ProgressSaveGameInstance->ProgressTotalPlayTime);
    CurrentFloorTime = FMath::Max(0.0, ProgressSaveGameInstance->ProgressCurrentFloorPlayTime);
    // 클리어 기록 복원
    LevelClearTimes.Empty();

    struct FClearSlotInfo
    {
        FString SlotName;
        ELevelClear Level;
    };

    const TArray<FClearSlotInfo> ClearSlots =
    {
        { ClearSecondLevelSlotName, ELevelClear::SecondFloorClear },
        { ClearSokobanSlotName, ELevelClear::SokobanClear },
        { ClearShootingSlotName, ELevelClear::ShootingGameClear },
        { ClearBossPhase1LevelSlotName, ELevelClear::BossFirstPhaseGameClear },
        { ClearBossPhase2LevelSlotName, ELevelClear::BossSecondPhaseGameClear }
    };

    for (const FClearSlotInfo& SlotInfo : ClearSlots)
    {
        UGTWClearHistorySaveGame* ClearSaveGameInstance =
            Cast<UGTWClearHistorySaveGame>(
                UGameplayStatics::LoadGameFromSlot(
                    SlotInfo.SlotName,
                    SaveGameInfoUserIndex));

        if (!IsValid(ClearSaveGameInstance))
        {
            continue;
        }

        const double* ClearTime =
            ClearSaveGameInstance->LevelClearTime.Find(SlotInfo.Level);

        if (!ClearTime)
        {
            continue;
        }

        LevelClearTimes.Add(SlotInfo.Level, *ClearTime);

        Debug::Print(FString::Printf(
            TEXT("클리어 기록 복원 : %s / %.2f"),
            *SlotInfo.SlotName,
            *ClearTime));
    }

    // 시간 갱신
    bIsTickable = true;
    OnTimeChanged.Broadcast(TotalPlayTime);

    // 저장된 레벨로 이동
    return OpenGameLevel(ProgressSaveGameInstance->ProgressLevel);
}

void UGTWGameInstance::ResetCurrentFloorTime()
{
    CurrentFloorTime = 0.0f;
}

bool UGTWGameInstance::IsLevelCleared(ELevelClear Level) const
{
    return LevelClearTimes.Contains(Level);
}

void UGTWGameInstance::CompleteGame()
{
    bIsTickable = false;
    
    if (!IsValid(ClearHistorySaveGame.Get()))
    {
        Debug::Print(TEXT("[GameInstance::CompleteGame] 클리어 시간은 확정됐으나 SaveGame 객체가 없어 저장 불가"));
        return;
    }

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

    switch (Level)
    {
    case ELevelClear::SecondFloorClear:
    {
        if (UGameplayStatics::DoesSaveGameExist(
            ClearSecondLevelSlotName, SaveGameInfoUserIndex))
        {
            if (UGTWClearHistorySaveGame* ClearSaveGameInstance =
                Cast<UGTWClearHistorySaveGame>(
                    UGameplayStatics::LoadGameFromSlot(
                        ClearSecondLevelSlotName,
                        SaveGameInfoUserIndex)))
            {
                ClearHistorySaveGame = ClearSaveGameInstance;
            }
            else
            {
                Debug::Print(TEXT("ClearSaveGameInstance Load 실패"));
                return;
            }
        }
        else
        {
            if (UGTWClearHistorySaveGame* ClearSaveGameInstance =
                Cast<UGTWClearHistorySaveGame>(
                    UGameplayStatics::CreateSaveGameObject(
                        UGTWClearHistorySaveGame::StaticClass())))
            {
                ClearHistorySaveGame = ClearSaveGameInstance;
            }
            else
            {
                Debug::Print(TEXT("ClearSaveGameInstance 객체 생성 실패"));
                return;
            }
        }

        // 로드 또는 생성 이후 값을 설정
        ClearHistorySaveGame->LevelClearTime.Add(Level, CurrentFloorTime);

        // 실제 파일 저장
        if (!UGameplayStatics::SaveGameToSlot(
            ClearHistorySaveGame,
            ClearSecondLevelSlotName,
            SaveGameInfoUserIndex))
        {
            Debug::Print(TEXT("ClearSecondLevelSlotName 저장 실패"));
            return;
        }

        Debug::Print(TEXT("ClearSecondLevelSlotName 저장 성공"));
    }
        break;
    case ELevelClear::BossFirstPhaseGameClear:
    {
        if (UGameplayStatics::DoesSaveGameExist(
            ClearBossPhase1LevelSlotName, SaveGameInfoUserIndex))
        {
            if (UGTWClearHistorySaveGame* ClearSaveGameInstance =
                Cast<UGTWClearHistorySaveGame>(
                    UGameplayStatics::LoadGameFromSlot(
                        ClearBossPhase1LevelSlotName,
                        SaveGameInfoUserIndex)))
            {
                ClearHistorySaveGame = ClearSaveGameInstance;
            }
            else
            {
                Debug::Print(TEXT("ClearSaveGameInstance Load 실패"));
                return;
            }
        }
        else
        {
            if (UGTWClearHistorySaveGame* ClearSaveGameInstance =
                Cast<UGTWClearHistorySaveGame>(
                    UGameplayStatics::CreateSaveGameObject(
                        UGTWClearHistorySaveGame::StaticClass())))
            {
                ClearHistorySaveGame = ClearSaveGameInstance;
            }
            else
            {
                Debug::Print(TEXT("ClearSaveGameInstance 객체 생성 실패"));
                return;
            }
        }

        // 로드 또는 생성 이후 값을 설정
        ClearHistorySaveGame->LevelClearTime.Add(Level, CurrentFloorTime);

        // 실제 파일 저장
        if (!UGameplayStatics::SaveGameToSlot(
            ClearHistorySaveGame,
            ClearBossPhase1LevelSlotName,
            SaveGameInfoUserIndex))
        {
            Debug::Print(TEXT("ClearBossPhase1LevelSlotName 저장 실패"));
            return;
        }

        Debug::Print(TEXT("ClearBossPhase1LevelSlotName 저장 성공"));
    }
        break;
    case ELevelClear::BossSecondPhaseGameClear:
    {
        if (UGameplayStatics::DoesSaveGameExist(
            ClearBossPhase2LevelSlotName, SaveGameInfoUserIndex))
        {
            if (UGTWClearHistorySaveGame* ClearSaveGameInstance =
                Cast<UGTWClearHistorySaveGame>(
                    UGameplayStatics::LoadGameFromSlot(
                        ClearBossPhase2LevelSlotName,
                        SaveGameInfoUserIndex)))
            {
                ClearHistorySaveGame = ClearSaveGameInstance;
            }
            else
            {
                Debug::Print(TEXT("ClearSaveGameInstance Load 실패"));
                return;
            }
        }
        else
        {
            if (UGTWClearHistorySaveGame* ClearSaveGameInstance =
                Cast<UGTWClearHistorySaveGame>(
                    UGameplayStatics::CreateSaveGameObject(
                        UGTWClearHistorySaveGame::StaticClass())))
            {
                ClearHistorySaveGame = ClearSaveGameInstance;
            }
            else
            {
                Debug::Print(TEXT("ClearSaveGameInstance 객체 생성 실패"));
                return;
            }
        }

        // 로드 또는 생성 이후 값을 설정
        ClearHistorySaveGame->LevelClearTime.Add(Level, CurrentFloorTime);

        // 실제 파일 저장
        if (!UGameplayStatics::SaveGameToSlot(
            ClearHistorySaveGame,
            ClearBossPhase2LevelSlotName,
            SaveGameInfoUserIndex))
        {
            Debug::Print(TEXT("ClearBossPhase1LevelSlotName 저장 실패"));
            return;
        }

        Debug::Print(TEXT("ClearBossPhase1LevelSlotName 저장 성공"));

        // 최종클리어 타임
        if (UGameplayStatics::DoesSaveGameExist(
            TotalPlayTimeSlotName, SaveGameInfoUserIndex))
        {
            if (UGTWClearHistorySaveGame* ClearSaveGameInstance =
                Cast<UGTWClearHistorySaveGame>(
                    UGameplayStatics::LoadGameFromSlot(
                        TotalPlayTimeSlotName,
                        SaveGameInfoUserIndex)))
            {
                ClearHistorySaveGame = ClearSaveGameInstance;
            }
            else
            {
                Debug::Print(TEXT("ClearSaveGameInstance Load 실패"));
                return;
            }
        }
        else
        {
            if (UGTWClearHistorySaveGame* ClearSaveGameInstance =
                Cast<UGTWClearHistorySaveGame>(
                    UGameplayStatics::CreateSaveGameObject(
                        UGTWClearHistorySaveGame::StaticClass())))
            {
                ClearHistorySaveGame = ClearSaveGameInstance;
            }
            else
            {
                Debug::Print(TEXT("ClearSaveGameInstance 객체 생성 실패"));
                return;
            }
        }

        // 로드 또는 생성 이후 값을 설정
        ClearHistorySaveGame->LevelClearTime.Add(Level, TotalPlayTime);

        // 실제 파일 저장
        if (!UGameplayStatics::SaveGameToSlot(
            ClearHistorySaveGame,
            TotalPlayTimeSlotName,
            SaveGameInfoUserIndex))
        {
            Debug::Print(TEXT("TotalPlayTimeSlotName 저장 실패"));
            return;
        }

        Debug::Print(TEXT("TotalPlayTimeSlotName 저장 성공"));

    }
        break;
    default:
        break;
    }
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

    LevelClearTimes.Add(Level, ClearTime);
    OnClearLevelChanged.Broadcast(Level, ClearTime);

    switch (Level)
    {
    case ELevelClear::SokobanClear:
    {
        if (UGameplayStatics::DoesSaveGameExist(
            ClearSokobanSlotName, SaveGameInfoUserIndex))
        {
            if (UGTWClearHistorySaveGame* ClearSaveGameInstance =
                Cast<UGTWClearHistorySaveGame>(
                    UGameplayStatics::LoadGameFromSlot(
                        ClearSokobanSlotName,
                        SaveGameInfoUserIndex)))
            {
                ClearHistorySaveGame = ClearSaveGameInstance;
            }
            else
            {
                Debug::Print(TEXT("ClearSaveGameInstance Load 실패"));
                return;
            }
        }
        else
        {
            if (UGTWClearHistorySaveGame* ClearSaveGameInstance =
                Cast<UGTWClearHistorySaveGame>(
                    UGameplayStatics::CreateSaveGameObject(
                        UGTWClearHistorySaveGame::StaticClass())))
            {
                ClearHistorySaveGame = ClearSaveGameInstance;
            }
            else
            {
                Debug::Print(TEXT("ClearSaveGameInstance 객체 생성 실패"));
                return;
            }
        }

        // 로드 또는 생성 이후 값을 설정
        ClearHistorySaveGame->LevelClearTime.Add(Level, ClearTime);

        // 실제 파일 저장
        if (!UGameplayStatics::SaveGameToSlot(
            ClearHistorySaveGame,
            ClearSokobanSlotName,
            SaveGameInfoUserIndex))
        {
            Debug::Print(TEXT("ClearSokobanSlotName 저장 실패"));
            return;
        }

        Debug::Print(TEXT("ClearSokobanSlotName 저장 성공"));

        break;
    }
    case ELevelClear::ShootingGameClear:
    {
        if (UGameplayStatics::DoesSaveGameExist(
            ClearShootingSlotName, SaveGameInfoUserIndex))
        {
            if (UGTWClearHistorySaveGame* ClearSaveGameInstance =
                Cast<UGTWClearHistorySaveGame>(
                    UGameplayStatics::LoadGameFromSlot(
                        ClearShootingSlotName,
                        SaveGameInfoUserIndex)))
            {
                ClearHistorySaveGame = ClearSaveGameInstance;
            }
            else
            {
                Debug::Print(TEXT("ClearSaveGameInstance Load 실패"));
                return;
            }
        }
        else
        {
            if (UGTWClearHistorySaveGame* ClearSaveGameInstance =
                Cast<UGTWClearHistorySaveGame>(
                    UGameplayStatics::CreateSaveGameObject(
                        UGTWClearHistorySaveGame::StaticClass())))
            {
                ClearHistorySaveGame = ClearSaveGameInstance;
            }
            else
            {
                Debug::Print(TEXT("ClearSaveGameInstance 객체 생성 실패"));
                return;
            }
        }

        // 로드 또는 생성 이후 값을 설정
        ClearHistorySaveGame->LevelClearTime.Add(Level, ClearTime);
        // 실제 파일 저장
        if (!UGameplayStatics::SaveGameToSlot(
            ClearHistorySaveGame,
            ClearShootingSlotName,
            SaveGameInfoUserIndex))
        {
            Debug::Print(TEXT("ClearShootingSlotName 저장 실패"));
            return;
        }

        Debug::Print(TEXT("ClearShootingSlotName 저장 성공"));
    }
    default:
        break;
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
        // 현재 플레이 중인 레벨과 누적 시간을 저장
        SaveProgressGame();
    }

    UGameplayStatics::OpenLevel(this, FName(LevelName));

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

// return: 저장 성공 시 true, 실패했거나 SaveGame이 Invalid 하면 false
bool UGTWGameInstance::SaveClearHistory()
{
    if (!IsValid(ClearHistorySaveGame.Get()))
    {
        return false;
    }

    return false;
}