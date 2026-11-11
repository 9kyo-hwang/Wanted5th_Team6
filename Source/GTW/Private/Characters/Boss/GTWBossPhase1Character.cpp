#include "Characters/Boss/GTWBossPhase1Character.h"

#include "Actor/Gimmicks/GTWBossHitObject.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Controllers/GTWBoss1PAIController.h"
#include "DrawDebugHelpers.h"
#include "Engine/TriggerBox.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Interfaces/GTWHitReactionReceiver.h"
#include "UI/GTWP1TornadoPresentationWidget.h"
#include "UI/GTWP1BossHitWidget.h"
#include "GTWGameInstance.h"
#include "HAL/FileManager.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void AGTWBossPhase1Character::LoadDifficultySettings()
{
	// 파일 오류 시 임의의 보너스를 주지 않고 기존 생성량을 유지한다.
	FastClearTimeThreshold = 0.f;
	FastClearObstacleRatio = 1.f;
	const FString DefaultFile = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("GTW/GameData/BossPhase1.ini"));
	const FString ExternalFile = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Config/BossPhase1.ini"));
	FString SettingsFile = ExternalFile;

	// GameData는 기존 UFS 패키징 대상이다. 원본을 읽어 수정 가능한 Saved 위치에 최초 한 번만 복사한다.
	if (!IFileManager::Get().FileExists(*ExternalFile))
	{
		FString Defaults;
		const bool bCopied = FFileHelper::LoadFileToString(Defaults, *DefaultFile)
			&& IFileManager::Get().MakeDirectory(*FPaths::GetPath(ExternalFile), true)
			&& FFileHelper::SaveStringToFile(Defaults, *ExternalFile, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
		if (!bCopied)
		{
			// 저장 폴더에 쓸 수 없어도 패키지 기본값으로 동작한다.
			SettingsFile = DefaultFile;
			UE_LOG(LogTemp, Warning, TEXT("P1: 외부 INI를 생성하지 못해 패키지 기본 INI를 사용합니다: %s"), *ExternalFile);
		}
	}

	// GConfig 캐시를 사용하지 않아 다음 P1 진입 때 파일 변경을 다시 읽는다.
	FConfigFile Settings;
	Settings.Read(SettingsFile);
	FString ThresholdText;
	FString RatioText;
	float Threshold = 0.f;
	float Ratio = 1.f;
	const bool bValid = Settings.GetString(TEXT("BossPhase1"), TEXT("FastClearTimeThreshold"), ThresholdText)
		&& Settings.GetString(TEXT("BossPhase1"), TEXT("FastClearObstacleRatio"), RatioText)
		&& LexTryParseString(Threshold, *ThresholdText) && LexTryParseString(Ratio, *RatioText)
		&& FMath::IsFinite(Threshold) && Threshold >= 0.f
		&& FMath::IsFinite(Ratio) && Ratio >= 0.f && Ratio <= 1.f;
	if (!bValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("P1: INI 값이 없거나 잘못되었습니다. 장애물 생성량을 보정하지 않습니다: %s"), *SettingsFile);
		return;
	}
	FastClearTimeThreshold = Threshold;
	FastClearObstacleRatio = Ratio;
	UE_LOG(LogTemp, Display, TEXT("P1 INI: %s (Threshold=%.2f, Ratio=%.2f)"),
		*FPaths::ConvertRelativePathToFull(SettingsFile), Threshold, Ratio);
}

void AGTWBossPhase1Character::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (!GetWorld()->IsGameWorld())
	{
		return;
	}

	LoadDifficultySettings();
	// BP의 원래 개수는 보존한다. 레벨 재시작 시에도 비율이 중복으로 적용되지 않는다.
	AdjustedAttackActorCount = FMath::Clamp(AttackActorCount, 1, 32);
	FourthFloorClearTime = -1.0;
	bHasFastClearBenefit = false;
	const UGTWGameInstance* GameInstance = GetGameInstance<UGTWGameInstance>();
	if (!GameInstance || !GameInstance->TryGetMinigameClearTime(EMinigameType::Shooting, FourthFloorClearTime))
	{
		return;
	}

	// 수치는 INI가 관리하고 C++은 기존 4층 기록 조회와 생성량 계산만 담당한다.
	bHasFastClearBenefit = FMath::IsFinite(FourthFloorClearTime)
		&& FastClearTimeThreshold > 0.f && FourthFloorClearTime < FastClearTimeThreshold;
	if (bHasFastClearBenefit)
	{
		AdjustedAttackActorCount = FMath::RoundToInt(AdjustedAttackActorCount * FMath::Clamp(FastClearObstacleRatio, 0.f, 1.f));
	}
}

void AGTWBossPhase1Character::StartCombat()
{
	StartEncounter();
}

void AGTWBossPhase1Character::StopCombat()
{
	StopEncounter();
}

/*
 * 읽는 순서: StartEncounter → AIController::RunAI → BP Service / BT Task.
 * Service는 추격을 요청하고, Task는 PlayDropObstacles를 호출한다.
 * BP는 에셋·수치·실행 순서, C++은 범위 계산·이동·공격 생성·정리를 담당한다.
 */
void AGTWBossPhase1Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateTornadoKickMotion();

	// 보스 이동 범위 제한
	EnforceArenaBounds();
	DrawArenaDebug();
}

void AGTWBossPhase1Character::EnforceArenaBounds()
{
	if (!IsValid(MovementBoundsActor) || !IsEncounterRunning())
	{
		return;
	}

	// 보스가 허용된 위치에 있는지 검사
	const FVector Confined = ConstrainPosition(GetActorLocation());
	// 레벨에 배치한 고정 Box 안으로 위치를 보정한다.
	if (!Confined.Equals(GetActorLocation(), 0.01f))
	{
		SetActorLocation(Confined, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

/* 보스 캡슐 크기에 대한 실제 이동 가능 범위 계산 */
FVector AGTWBossPhase1Character::GetMovementExtent() const
{
	const FTransform Transform = GetMovementBounds()->GetComponentTransform();
	const FVector Extent = GetMovementBounds()->GetUnscaledBoxExtent();
	const FVector Scale = Transform.GetScale3D().GetAbs();
	const float Radius = GetCapsuleComponent()->GetScaledCapsuleRadius();

	const float X = FMath::Max(0.f, Extent.X - Radius / FMath::Max(Scale.X, KINDA_SMALL_NUMBER));
	const float Y = FMath::Max(0.f, Extent.Y - Radius / FMath::Max(Scale.Y, KINDA_SMALL_NUMBER));

	return FVector(X, Y, Extent.Z);
}

/* 현재/다음 위치를 각 축으로 제한한다. GetChaseLocation의 방향 보존 계산과 목적이 다르다. */
FVector AGTWBossPhase1Character::ConstrainPosition(const FVector& Position) const
{
	const FTransform Transform = GetMovementBounds()->GetComponentTransform();
	const FVector Extent = GetMovementExtent();

	FVector Local = Transform.InverseTransformPosition(Position);
	Local.X = FMath::Clamp(Local.X, -Extent.X, Extent.X);
	Local.Y = FMath::Clamp(Local.Y, -Extent.Y, Extent.Y);
	FVector Result = Transform.TransformPosition(Local);
	Result.Z = BossHeight;

	return Result;
}

FVector AGTWBossPhase1Character::GetChaseLocation(const FVector& PlayerPosition) const
{
	const FTransform Transform = GetMovementBounds()->GetComponentTransform();
	const FVector Extent = GetMovementExtent();
	FVector Local = Transform.InverseTransformPosition(PlayerPosition);

	// 목표는 중심에서 플레이어를 향하는 방향을 유지한다. 현재 위치 보정(ConstrainPosition)과 구분한다.
	float Factor = 1.f;
	const bool bOutsideXBounds = FMath::Abs(Local.X) > Extent.X;
	const bool bOutsideYBounds = FMath::Abs(Local.Y) > Extent.Y;

	if (bOutsideXBounds)
	{
		Factor = FMath::Min(Factor, Extent.X / FMath::Abs(Local.X));
	}
	if (bOutsideYBounds)
	{
		Factor = FMath::Min(Factor, Extent.Y / FMath::Abs(Local.Y));
	}
	Local.X *= Factor;
	Local.Y *= Factor;
	FVector Result = Transform.TransformPosition(Local);
	Result.Z = BossHeight;

	return Result;
}

void AGTWBossPhase1Character::UpdatePursuit(APawn* TargetPlayer, const FVector& ChaseLocation, float DeltaSeconds)
{
	if (bReactingToHit || !IsValid(MovementBoundsActor) || !IsEncounterRunning() || !IsValid(TargetPlayer))
	{
		return;
	}

	// 시선은 실제 플레이어를 향하고, 이동은 BP가 전달한 사각형 내부 목표를 향한다.
	const FVector Target = TargetPlayer->GetActorLocation();
	const FVector Direction = (Target - GetActorLocation()) * FVector(1, 1, 0);

	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(Direction.Rotation());
	}

	if (Direction.SizeSquared() > FMath::Square(FMath::Max(0.f, StopDistance)))
	{
		const FVector Next = FMath::VInterpConstantTo(GetActorLocation(), ChaseLocation, DeltaSeconds, FMath::Max(0.f, ChaseSpeed));
		SetActorLocation(ConstrainPosition(Next), true);
	}
}

void AGTWBossPhase1Character::PlayDropObstacles(APawn* Player)
{
	// 공격 가능 여부는 보스 한 곳에서 판단한다. Task는 대상 전달만 담당한다.
	if (!bRunning || bReactingToHit || !IsValid(MovementBoundsActor) || !IsValid(Player) || IsPreparingAttack())
	{
		return;
	}
	// P2와 같은 Boss 함수 -> 공통 몽타주 재생 흐름. P1은 모션 정상 종료 후 실제 공격을 실행한다.
	if (!PlayAttackAnim(DropAttack, Player))
	{
		SpawnAttackActors(Player);
	}
}

bool AGTWBossPhase1Character::PlayTornadoKick()
{
	if (!bRunning || bReactingToHit || !IsValid(MovementBoundsActor) || IsPreparingAttack() || !TornadoKick)
	{
		return false;
	}

	// 공통 몽타주 재생/종료 처리를 재사용한다. 킥은 기믹 생성 공격과 구분한다.
	if (!PlayAttackAnim(TornadoKick, nullptr))
	{
		return false;
	}
	bTornadoKickPlaying = true;
	TornadoMeshBaseLocation = GetMesh()->GetRelativeLocation();
	bTornadoPlayerHit = false;
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0))
	{
		UClass* PresentationClass = TornadoPresentationClass ? TornadoPresentationClass.Get() : UGTWP1TornadoPresentationWidget::StaticClass();
		if (UGTWP1TornadoPresentationWidget* Presentation = CreateWidget<UGTWP1TornadoPresentationWidget>(PlayerController, PresentationClass))
		{
			TornadoPresentation = Presentation;
			Presentation->SetBoss(this);
			Presentation->AddToViewport(20);
		}
	}
	return true;
}

void AGTWBossPhase1Character::UpdateTornadoKickMotion()
{
	UAnimInstance* Instance = GetMesh()->GetAnimInstance();
	if (!bTornadoKickPlaying || !Instance || !TornadoKick)
	{
		return;
	}

	const float Position = Instance->Montage_GetPosition(TornadoKick);
	const int32 SectionIndex = TornadoKick->GetSectionIndexFromPosition(Position);
	if (SectionIndex == INDEX_NONE)
	{
		return;
	}
	float StartTime = 0.f;
	float EndTime = 0.f;
	TornadoKick->GetSectionStartAndEndTime(SectionIndex, StartTime, EndTime);
	const FName Section = TornadoKick->GetSectionName(SectionIndex);
	const float Alpha = FMath::Clamp((Position - StartTime) / FMath::Max(EndTime - StartTime, KINDA_SMALL_NUMBER), 0.f, 1.f);
	const float SmoothAlpha = FMath::SmoothStep(0.f, 1.f, Alpha);
	const bool bAirKick = Section == TEXT("AirKick");
	const float HeightAlpha = bAirKick ? 1.f : (Section == TEXT("Land") ? 1.f - SmoothAlpha : SmoothAlpha);

	// 좌표 제한은 액터가 담당한다. 시각적인 추가 점프 높이는 메시 오프셋으로 분리한다.
	const float ParentScaleZ = FMath::Max(FMath::Abs(GetActorScale3D().Z), KINDA_SMALL_NUMBER);
	GetMesh()->SetRelativeLocation(TornadoMeshBaseLocation + FVector::UpVector * (FMath::Max(0.f, TornadoKickExtraHeight) * HeightAlpha / ParentScaleZ));
	// 타임라인/Notify 위치는 바꾸지 않고 재생 시계만 조절해 판정 구간이 모션을 따라가게 한다.
	Instance->Montage_SetPlayRate(TornadoKick, bAirKick ? FMath::Clamp(TornadoKickPlayRate, 0.1f, 5.f) : 1.f);
}

void AGTWBossPhase1Character::ResetTornadoKickMotion()
{
	if (TornadoPresentation)
	{
		TornadoPresentation->RemoveFromParent();
		TornadoPresentation = nullptr;
	}
	GetMesh()->SetRelativeLocation(TornadoMeshBaseLocation);
	if (UAnimInstance* Instance = GetMesh()->GetAnimInstance())
	{
		Instance->Montage_SetPlayRate(TornadoKick, 1.f);
	}
}

void AGTWBossPhase1Character::BeginTornadoKickDamage()
{
	if (!bRunning || !bTornadoKickPlaying)
	{
		return;
	}

	const FName Bones[] = { LeftKickKnee, LeftKickFoot, RightKickKnee, RightKickFoot };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Bones); ++Index)
	{
		if (!GetMesh()->DoesSocketExist(Bones[Index]))
		{
			UE_LOG(LogTemp, Warning, TEXT("P1 TornadoKick: missing leg bone %s"), *Bones[Index].ToString());
			return;
		}
		PreviousKickBones[Index] = GetMesh()->GetSocketLocation(Bones[Index]);
	}
	bTornadoDamageActive = true;
	if (!KickPoseFinalizedHandle.IsValid())
	{
		// Notify 시점뿐 아니라 최종 애니메이션 포즈가 확정된 뒤에도 실제 다리를 검사한다.
		KickPoseFinalizedHandle = GetMesh()->RegisterOnBoneTransformsFinalizedDelegate(
			FOnBoneTransformsFinalizedMultiCast::FDelegate::CreateUObject(this, &ThisClass::UpdateTornadoKickDamage));
	}
}

void AGTWBossPhase1Character::UpdateTornadoKickDamage()
{
	if (!bRunning || !bTornadoKickPlaying || !bTornadoDamageActive)
	{
		return;
	}

	const FName Bones[] = { LeftKickKnee, LeftKickFoot, RightKickKnee, RightKickFoot };
	FVector CurrentBones[4];
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Bones); ++Index)
	{
		CurrentBones[Index] = GetMesh()->GetSocketLocation(Bones[Index]);
		// 고속 회전 중 프레임 사이로 지나간 무릎/발의 경로도 검사한다.
		TraceTornadoKick(PreviousKickBones[Index], CurrentBones[Index]);
		PreviousKickBones[Index] = CurrentBones[Index];
	}
	TraceTornadoKick(CurrentBones[0], CurrentBones[1]);
	TraceTornadoKick(CurrentBones[2], CurrentBones[3]);
	// 무릎의 부모 뼈(허벅지)까지 포함해 다리 전체에 판정을 둔다.
	for (const int32 KneeIndex : { 0, 2 })
	{
		const FName Thigh = GetMesh()->GetParentBone(Bones[KneeIndex]);
		if (!Thigh.IsNone())
		{
			TraceTornadoKick(GetMesh()->GetSocketLocation(Thigh), CurrentBones[KneeIndex]);
		}
	}
}

void AGTWBossPhase1Character::TraceTornadoKick(const FVector& Start, const FVector& End)
{
	const float Radius = FMath::Max(1.f, TornadoKickRadius);
#if !UE_BUILD_SHIPPING
	if (bDrawDebug)
	{
		const FVector Delta = End - Start;
		DrawDebugCapsule(GetWorld(), (Start + End) * 0.5f, Delta.Size() * 0.5f + Radius, Radius,
			FQuat::FindBetweenNormals(FVector::UpVector, Delta.IsNearlyZero() ? FVector::UpVector : Delta.GetSafeNormal()), FColor::Red, false, 0.05f);
	}
#endif
	if (bTornadoPlayerHit)
	{
		return;
	}
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!IsValid(Player))
	{
		return;
	}

	const ACharacter* PlayerCharacter = Cast<ACharacter>(Player);
	if (!PlayerCharacter)
	{
		return;
	}

	// 단일 플레이어의 캡슐과 다리 구간을 직접 비교한다.
	// 래그돌 등으로 메시의 충돌 채널이 바뀌어도 동일한 접촉 범위를 사용한다.
	const UCapsuleComponent* Capsule = PlayerCharacter->GetCapsuleComponent();
	const float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	const float HalfSegment = FMath::Max(0.f, Capsule->GetScaledCapsuleHalfHeight() - CapsuleRadius);
	const FVector CapsuleOffset = Capsule->GetUpVector() * HalfSegment;
	const FVector CapsuleCenter = Capsule->GetComponentLocation();
	FVector LegPoint;
	FVector PlayerPoint;
	FMath::SegmentDistToSegmentSafe(Start, End, CapsuleCenter - CapsuleOffset, CapsuleCenter + CapsuleOffset, LegPoint, PlayerPoint);
	const float ContactRadius = Radius + CapsuleRadius;
	const bool bTouchesPlayer = FVector::DistSquared(LegPoint, PlayerPoint) <= FMath::Square(ContactRadius);
	if (!bTouchesPlayer)
	{
		return;
	}

	if (IGTWHitReactionReceiver* Receiver = Cast<IGTWHitReactionReceiver>(Player))
	{
		FVector Direction = (CapsuleCenter - GetActorLocation()).GetSafeNormal2D();
		if (Direction.IsNearlyZero())
		{
			Direction = GetActorForwardVector().GetSafeNormal2D();
		}
		// 지면에서 수평 충격이 바로 감쇠되지 않도록 약간 들어 올린다.
		// 기존 인터페이스가 충격·래그돌·무적 시간을 처리한다. 공격당 한 번만 전달한다.
		Direction.Z = 0.35f;
		bTornadoPlayerHit = true;
		Receiver->HitKnockbackObject(Direction.GetSafeNormal(), TornadoKickPower);
		// P1 직접 타격은 지면 마찰과 스케이트 이동 상태에 관계없이 밀려나게 한다.
		// 기존 인터페이스의 피격 반응을 재사용하고, 이동 속도만 Character API로 확정한다.
		if (ACharacter* HitCharacter = Cast<ACharacter>(Player))
		{
			HitCharacter->LaunchCharacter(Direction.GetSafeNormal() * TornadoKickPower, true, true);
		}
	}
}

void AGTWBossPhase1Character::EndTornadoKickDamage()
{
	bTornadoDamageActive = false;
	if (KickPoseFinalizedHandle.IsValid())
	{
		GetMesh()->UnregisterOnBoneTransformsFinalizedDelegate(KickPoseFinalizedHandle);
		KickPoseFinalizedHandle.Reset();
	}
}

void AGTWBossPhase1Character::CancelTornadoKick()
{
	if (bTornadoKickPlaying)
	{
		CancelPreparedAttack();
	}
}

// 기믹 BP와 공격 수치는 BP_P1Boss에서 편집한다.
TArray<TSubclassOf<AActor>> AGTWBossPhase1Character::GetAttackClasses() const
{
	TArray<TSubclassOf<AActor>> Classes;
	Classes.Reserve(AttackActorClasses.Num());

	for (const auto& Class : AttackActorClasses)
	{
		if (Class && !Class->HasAnyClassFlags(CLASS_Abstract))
		{
			Classes.Add(Class);
		}
	}
	return Classes;
}

// 플레이어 주변 한 지점을 선택
// 지면 기믹은 바닥을 찾은 경우에만
bool AGTWBossPhase1Character::FindAttackSpawnLocation(APawn* Player, const FCollisionQueryParams& GroundQuery, FVector& OutLocation) const
{
	// 1. 플레이어 반경 안에서 무작위 XY 위치를 선택
	const FVector PlayerLocation = Player->GetActorLocation();
	const FVector2D RandomOffset = FMath::RandPointInCircle(FMath::Max(1.f, PlayerRadius));
	const FVector CandidateLocation = PlayerLocation + FVector(RandomOffset.X, RandomOffset.Y, 0.f);

	// 2. 후보 지점의 위에서 플레이어 발밑 근처까지 바닥을 검사
	constexpr float TraceStartHeight = 100.f;
	constexpr float SpawnHeightOffset = 2.f;
	constexpr float MinimumGroundNormalZ = 0.5f;

	float PlayerFeetZ = PlayerLocation.Z;
	if (const ACharacter* Character = Cast<ACharacter>(Player))
	{
		PlayerFeetZ -= Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}

	const FVector TraceStart(CandidateLocation.X, CandidateLocation.Y, PlayerLocation.Z + TraceStartHeight);
	const FVector TraceEnd(CandidateLocation.X, CandidateLocation.Y, PlayerFeetZ - FMath::Max(1.f, GroundSearchDepth));

	// 제외 목록은 공격마다 한 번 준비하여 모든 후보 검색에서 재사용한다.
	FCollisionObjectQueryParams GroundTypes;
	GroundTypes.AddObjectTypesToQuery(ECC_WorldStatic);
	GroundTypes.AddObjectTypesToQuery(ECC_WorldDynamic);

	FHitResult GroundHit;
	const bool bFoundGround = GetWorld()->LineTraceSingleByObjectType(GroundHit, TraceStart, TraceEnd, GroundTypes, GroundQuery);
	if (!bFoundGround)
	{
		return false;
	}
	if (GroundHit.ImpactNormal.Z < MinimumGroundNormalZ)
	{
		return false;
	}

	// 3. 바닥에 묻히지 않도록 살짝 위에 생성한다. 실패 시 호출자가 다른 후보를 시도한다.
	OutLocation = GroundHit.ImpactPoint + FVector::UpVector * SpawnHeightOffset;
	return true;
}

// 수치는 BP_P1Boss에서 편집하고, C++은 순차 생성과 종료 정리만 담당한다.
void AGTWBossPhase1Character::SpawnAttackActors(APawn* Player)
{
	if (!bRunning || !IsValid(Player) || GetWorldTimerManager().IsTimerActive(AttackSpawnTimer))
	{
		return;
	}
	AttackSpawnTarget = Player;
	RemainingAttackActors = AdjustedAttackActorCount;
	if (RemainingAttackActors <= 0)
	{
		AttackSpawnTarget.Reset();
		return;
	}
	SpawnNextAttackActor();
	if (RemainingAttackActors > 0)
	{
		GetWorldTimerManager().SetTimer(AttackSpawnTimer, this, &ThisClass::SpawnNextAttackActor,
			FMath::Max(0.05f, AttackSpawnInterval), true);
	}
}

void AGTWBossPhase1Character::SpawnNextAttackActor()
{
	APawn* Player = AttackSpawnTarget.Get();
	// 실패한 위치 검색도 한 회로 센다. 대상 소멸·전투 종료 시 타이머를 남기지 않는다.
	if (!bRunning || !IsValid(Player) || RemainingAttackActors <= 0)
	{
		GetWorldTimerManager().ClearTimer(AttackSpawnTimer);
		AttackSpawnTarget.Reset();
		RemainingAttackActors = 0;
		return;
	}
	if (--RemainingAttackActors == 0)
	{
		GetWorldTimerManager().ClearTimer(AttackSpawnTimer);
	}

	if (!IsValid(MovementBoundsActor) || !IsEncounterRunning() || !IsValid(Player))
	{
		return;
	}

	const TArray<TSubclassOf<AActor>> Classes = GetAttackClasses();
	if (Classes.IsEmpty())
	{
		return;
	}
	SpawnedAttackActors.RemoveAll([](const TWeakObjectPtr<AActor>& Actor)
	{
		return !Actor.IsValid();
	});

	FCollisionQueryParams GroundQuery(SCENE_QUERY_STAT(P1AttackGround), true, this);
	GroundQuery.AddIgnoredActor(Player);
	for (const auto& Existing : SpawnedAttackActors)
	{
		GroundQuery.AddIgnoredActor(Existing.Get());
	}
	// 타격 액터의 접촉 구체를 바닥으로 오인하지 않도록 제외한다.
	for (const auto& Object : SpawnedHitObjects)
	{
		GroundQuery.AddIgnoredActor(Object.Get());
	}

	// 여기서는 클래스 선택과 생성만 처리한다.
	// 운석 낙하·번개 예고·넉백 등은 생성된 기믹 BP/C++의 기존 동작이다.
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const float Scale = FMath::Max(0.1f, GroundAttackScale);
	const int32 DesiredCount = 1;
	const int32 MaxAttempts = DesiredCount * 10;
	int32 SpawnCount = 0;

	for (int32 Attempt = 0; Attempt < MaxAttempts && SpawnCount < DesiredCount; ++Attempt)
	{
		FVector Location;
		if (!FindAttackSpawnLocation(Player, GroundQuery, Location))
		{
			continue;
		}

		const FTransform Transform(FRotator::ZeroRotator, Location, FVector(Scale));
		AActor* Attack = GetWorld()->SpawnActor<AActor>(Classes[FMath::RandHelper(Classes.Num())], Transform, Params);
		if (!Attack)
		{
			continue;
		}
		if (Attack->GetLifeSpan() <= 0.f)
		{
			Attack->SetLifeSpan(FMath::Max(0.1f, AttackActorLifeSpan));
		}

		SpawnedAttackActors.Add(Attack);
		GroundQuery.AddIgnoredActor(Attack);
		++SpawnCount;
	}
}

// 타격 액터의 접촉/비행은 기존 GTWBossHitObject가 처리한다.
// P1 보스는 생성, 목표 지정, 적중 알림 연결과 전투 종료 정리만 담당한다.
void AGTWBossPhase1Character::SpawnHitObject()
{
	if (!bRunning || !HitObjectClass)
	{
		return;
	}
	SpawnedHitObjects.RemoveAll([](const TWeakObjectPtr<AGTWBossHitObject>& Object)
	{
		return !Object.IsValid();
	});
	if (SpawnedHitObjects.Num() >= FMath::Clamp(MaxHitObjectCount, 1, 10))
	{
		return;
	}
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!IsValid(Player))
	{
		return;
	}

	FCollisionQueryParams GroundQuery(SCENE_QUERY_STAT(P1HitObjectGround), true, this);
	GroundQuery.AddIgnoredActor(Player);
	GroundQuery.AddIgnoredActor(MovementBoundsActor);
	for (const auto& Object : SpawnedHitObjects)
	{
		GroundQuery.AddIgnoredActor(Object.Get());
	}
	for (const auto& Attack : SpawnedAttackActors)
	{
		GroundQuery.AddIgnoredActor(Attack.Get());
	}

	for (int32 Attempt = 0; Attempt < 20; ++Attempt)
	{
		FVector Location;
		if (!FindAttackSpawnLocation(Player, GroundQuery, Location))
		{
			continue;
		}
		// 생성 즉시 플레이어에게 닿지 않게 떨어뜨리고, 바닥 위에 보이도록 띄운다.
		if (FVector::DistSquared2D(Location, Player->GetActorLocation()) < FMath::Square(250.f))
		{
			continue;
		}
		Location.Z += 150.f;
		const FTransform Transform(FRotator::ZeroRotator, Location);
		AGTWBossHitObject* Object = GetWorld()->SpawnActorDeferred<AGTWBossHitObject>(
			HitObjectClass, Transform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		if (!Object)
		{
			return;
		}
		// BeginPlay의 접촉 감지가 켜지기 전에 목표와 적중 콜백을 먼저 연결한다.
		Object->SetTarget(this);
		Object->OnHitBoss.BindUObject(this, &ThisClass::OnHitObjectReachedBoss);
		SpawnedHitObjects.Add(Object);
		UGameplayStatics::FinishSpawningActor(Object, Transform);
		if (IsValid(Object))
		{
			Object->SetLifeSpan(FMath::Max(5.f, HitObjectLifeSpan));
		}
		return;
	}
}

void AGTWBossPhase1Character::OnHitObjectReachedBoss(AGTWBossHitObject* HitObject)
{
	if (!bRunning || !SpawnedHitObjects.Contains(HitObject))
	{
		return;
	}
	// 먼저 목록에서 제거해 같은 액터의 중복 알림과 종료 시 중복 정리를 막는다.
	SpawnedHitObjects.Remove(HitObject);
	DisplayedHitCount = FMath::Min(DisplayedHitCount + 1, 3);
	++PendingHitNotifications;
	if (AGTWBoss1PAIController* AI = Cast<AGTWBoss1PAIController>(GetController()))
	{
		AI->StopAI();
	}
	CancelPreparedAttack();
	bReactingToHit = true;
	GetWorldTimerManager().PauseTimer(AttackSpawnTimer);
	if (UAnimInstance* Anim = GetMesh()->GetAnimInstance(); Anim && HitReaction)
	{
		const float Rate = HitReaction->GetPlayLength() / FMath::Max(0.1f, HitReactionDuration);
		if (Anim->Montage_Play(HitReaction, Rate) > 0.f)
		{
			HitReactionMontage = HitReaction;
		}
	}
	// 연속 적중은 HUD에 즉시 표시하되 마지막 반응까지 보여준 뒤 기존 카운트에 전달한다.
	GetWorldTimerManager().SetTimer(HitReactionTimer, this, &ThisClass::FinishHitReaction, FMath::Max(0.1f, HitReactionDuration), false);
}

void AGTWBossPhase1Character::FinishHitReaction()
{
	bReactingToHit = false;
	HitReactionMontage = nullptr;
	const int32 Notifications = PendingHitNotifications;
	PendingHitNotifications = 0;
	for (int32 Index = 0; Index < Notifications && bRunning; ++Index)
	{
		OnObjectHit.Broadcast();
	}
	if (bRunning)
	{
		GetWorldTimerManager().UnPauseTimer(AttackSpawnTimer);
		if (AGTWBoss1PAIController* AI = Cast<AGTWBoss1PAIController>(GetController()))
		{
			AI->RunAI();
		}
	}
}

bool AGTWBossPhase1Character::PlayAttackAnim(UAnimMontage* Montage, APawn* Player)
{
	if (bReactingToHit)
	{
		return false;
	}
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!Montage || !AnimInstance)
	{
		return false;
	}

	// DefaultSlot이 ABP의 Output Pose에 연결되어야 몽타주가 화면에 보인다.
	if (AnimInstance->Montage_Play(Montage) <= 0.f)
	{
		return false;
	}

	PreparedTarget = Player;
	PreparedMontage = Montage;
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &ThisClass::OnAttackMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, Montage);
	return true;
}

void AGTWBossPhase1Character::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != PreparedMontage)
	{
		return;
	}

	APawn* Player = PreparedTarget.Get();
	const bool bWasTornadoKick = bTornadoKickPlaying;
	if (bWasTornadoKick)
	{
		ResetTornadoKickMotion();
	}
	EndTornadoKickDamage();
	bTornadoKickPlaying = false;
	PreparedTarget.Reset();
	PreparedMontage = nullptr;

	// 현재 공격 시점은 Notify가 아니라 몽타주 정상 종료다.
	// 피격 등으로 모션이 중단되면 공격을 생성하지 않는다.
	if (!bInterrupted && !bWasTornadoKick)
	{
		SpawnAttackActors(Player);
	}
}

void AGTWBossPhase1Character::CancelPreparedAttack()
{
	if (bTornadoKickPlaying)
	{
		ResetTornadoKickMotion();
	}
	EndTornadoKickDamage();
	bTornadoKickPlaying = false;
	UAnimMontage* Montage = PreparedMontage;
	PreparedTarget.Reset();
	PreparedMontage = nullptr;
	if (Montage)
	{
		if (UAnimInstance* Instance = GetMesh()->GetAnimInstance())
		{
			FOnMontageEnded Empty;
			Instance->Montage_SetEndDelegate(Empty, Montage);
			Instance->Montage_Stop(Montage->GetDefaultBlendOutTime(), Montage);
		}
	}
}

void AGTWBossPhase1Character::StopEncounter()
{
	if (!bRunning)
	{
		return;
	}
	// 늦게 도착한 몽타주 종료 콜백도 공격할 수 없도록 먼저 종료 상태로 바꾼다.
	bRunning = false;
	GetWorldTimerManager().ClearTimer(HitReactionTimer);
	bReactingToHit = false;
	PendingHitNotifications = 0;
	if (HitReactionMontage)
	{
		if (UAnimInstance* Anim = GetMesh()->GetAnimInstance())
		{
			Anim->Montage_Stop(0.1f, HitReactionMontage);
		}
		HitReactionMontage = nullptr;
	}
	if (HitWidget)
	{
		HitWidget->RemoveFromParent();
		HitWidget = nullptr;
	}
	if (AGTWBoss1PAIController* AI = Cast<AGTWBoss1PAIController>(GetController()))
	{
		AI->StopAI();
	}
	CancelPreparedAttack();
	GetWorldTimerManager().ClearTimer(HitObjectSpawnTimer);
	GetWorldTimerManager().ClearTimer(AttackSpawnTimer);
	AttackSpawnTarget.Reset();
	RemainingAttackActors = 0;
	for (const auto& Object : SpawnedHitObjects)
	{
		if (Object.IsValid())
		{
			Object->OnHitBoss.Unbind();
			Object->Destroy();
		}
	}
	SpawnedHitObjects.Reset();
	for (const auto& Actor : SpawnedAttackActors)
	{
		if (Actor.IsValid())
		{
			Actor->Destroy();
		}
	}
	SpawnedAttackActors.Reset();
	GetCharacterMovement()->SetMovementMode(static_cast<EMovementMode>(PreviousMovementMode), PreviousCustomMode);
}

void AGTWBossPhase1Character::EndPlay(const EEndPlayReason::Type Reason)
{
	StopEncounter();
	Super::EndPlay(Reason);
}

/* 전투 시작 */
void AGTWBossPhase1Character::StartEncounter()
{
	// 전투중인지 확인
	if (bRunning || !IsValid(MovementBoundsActor) || !IsValid(GetMovementBounds()))
	{
		return;
	}

	// 보스가 특정 범위 안에 있는지 확인
	BossHeight = GetActorLocation().Z;
	if (!GetActorLocation().Equals(ConstrainPosition(GetActorLocation()), 1.f))
	{
		UE_LOG(LogTemp, Warning, TEXT("P1: 보스 범위 안에 없음"));
		return;
	}

	if (!GetController())
	{
		SpawnDefaultController();
	}

	AGTWBoss1PAIController* AI = Cast<AGTWBoss1PAIController>(GetController());
	if (!AI)
	{
		return;
	}

	// 시퀀서 미리보기의 Custom Mode가 PIE에 복사될 수 있다.
	// 인트로가 끝난 뒤에는 BP에서 지정한 Anim Class로 제어를 돌려 공격 Slot이 평가되게 한다.
	USkeletalMeshComponent* BossMesh = GetMesh();
	if (BossMesh->GetAnimationMode() != EAnimationMode::AnimationBlueprint)
	{
		BossMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	}

	// 이동 주체는 UpdatePursuit 한 곳이다. CharacterMovement와 동시에 이동하지 않게 한다.
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	PreviousMovementMode = Movement->MovementMode;
	PreviousCustomMode = Movement->CustomMovementMode;
	Movement->StopMovementImmediately();
	Movement->DisableMovement();
	bRunning = true;
	DisplayedHitCount = 0;
	PendingHitNotifications = 0;
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		HitWidget = HitStatusWidgetClass ? CreateWidget<UGTWP1BossHitWidget>(PC, HitStatusWidgetClass) : nullptr;
		if (HitWidget)
		{
			HitWidget->SetBoss(this);
			HitWidget->AddToViewport(15);
		}
	}
	if (!AI->RunAI())
	{
		StopEncounter();
		return;
	}
	if (HitObjectClass)
	{
		SpawnHitObject();
		GetWorldTimerManager().SetTimer(HitObjectSpawnTimer, this, &ThisClass::SpawnHitObject,
			FMath::Max(0.5f, HitObjectSpawnInterval), true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("P1: BP_P1Boss의 HitObjectClass에 보스 타격 BP를 지정해야 합니다."));
	}
}

void AGTWBossPhase1Character::DrawArenaDebug() const
{
#if !UE_BUILD_SHIPPING
	if (!bRunning || !IsValid(MovementBoundsActor) || !bDrawDebug)
	{
		return;
	}
	const UBoxComponent* Bounds = GetMovementBounds();
	DrawDebugBox(GetWorld(), Bounds->GetComponentLocation(), Bounds->GetScaledBoxExtent(), Bounds->GetComponentQuat(), FColor::Cyan, false, -1.f, 0, 3.f);
	if (const APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		DrawDebugCircle(GetWorld(), Player->GetActorLocation() + FVector(0, 0, 10), FMath::Max(1.f, PlayerRadius), 64, FColor::Yellow, false, -1.f, 0, 3.f, FVector::ForwardVector, FVector::RightVector, false);
	}
#endif
}

UBoxComponent* AGTWBossPhase1Character::GetMovementBounds() const
{
	return IsValid(MovementBoundsActor) ? Cast<UBoxComponent>(MovementBoundsActor->GetCollisionComponent()) : nullptr;
}
