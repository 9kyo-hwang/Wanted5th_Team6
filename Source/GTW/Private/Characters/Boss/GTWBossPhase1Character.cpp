#include "Characters/Boss/GTWBossPhase1Character.h"

#include "Actor/Gimmicks/GTWBossHitObject.h"
#include "Actor/Gimmicks/GTWMeteorGimmick.h"
#include "Actor/Gimmicks/GTWP1TornadoJumpPad.h"
#include "TimerManager.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Controllers/GTWBoss1PAIController.h"
#include "DrawDebugHelpers.h"
#include "Engine/TriggerBox.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
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
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AGTWBossPhase1Character::AGTWBossPhase1Character()
{
	// CDO에 에셋 참조를 보관해 패키징에도 포함하고, 생성할 때마다 다시 로드하지 않는다.
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> MaterialRef(
		TEXT("/Game/Assets/QRCode/QRCode_Mat.QRCode_Mat"));
	if (MaterialRef.Succeeded())
	{
		HitObjectMaterial = MaterialRef.Object;
	}
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DisplayMeshRef(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (DisplayMeshRef.Succeeded())
	{
		HitObjectDisplayMesh = DisplayMeshRef.Object;
	}
}

namespace
{
	// 처음에만 배포 INI를 Saved로 복사한다. 이후 레벨 진입부터는 사용자가 편집한 파일을 읽는다.
	FString GetP1SettingsFile(const TCHAR* FileName)
	{
		const FString DefaultFile = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("GTW/GameData"), FileName);
		const FString ExternalFile = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Config"), FileName);
		if (IFileManager::Get().FileExists(*ExternalFile))
		{
			return ExternalFile;
		}
		FString Contents;
		const bool bCopied = FFileHelper::LoadFileToString(Contents, *DefaultFile)
			&& IFileManager::Get().MakeDirectory(*FPaths::GetPath(ExternalFile), true)
			&& FFileHelper::SaveStringToFile(Contents, *ExternalFile, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
		if (bCopied)
		{
			return ExternalFile;
		}
		UE_LOG(LogTemp, Warning, TEXT("P1: %s를 복사하지 못해 배포 기본값을 사용합니다."), FileName);
		return DefaultFile;
	}
}

void AGTWBossPhase1Character::LoadBossSettings()
{
	// 보너스 기준과 제출 목표는 INI에서만 관리한다. 장애물 생성량에는 영향을 주지 않는다.
	FastClearTimeThreshold = 0.f;
	FastClearSubmissionReductionCount = 0;
	const FString DefaultFile = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("GTW/GameData/BossPhase1.ini"));
	const FString SettingsFile = GetP1SettingsFile(TEXT("BossPhase1.ini"));

	// GConfig 캐시를 사용하지 않아 다음 P1 진입 때 파일 변경을 다시 읽는다.
	FConfigFile Settings;
	Settings.Read(SettingsFile);
	FConfigFile Defaults;
	Defaults.Read(DefaultFile);
	// 예전 외부 INI에 새 항목이 없으면 배포 기본 INI를 사용한다. BP 수치는 읽지 않는다.
	auto ReadNumber = [&](const TCHAR* Key, float Minimum, float Maximum, float SafeValue)
	{
		FString Text;
		if (!Settings.GetString(TEXT("BossPhase1"), Key, Text))
		{
			Defaults.GetString(TEXT("BossPhase1"), Key, Text);
		}
		float Value = SafeValue;
		if (!LexTryParseString(Value, *Text) || !FMath::IsFinite(Value) || Value < Minimum || Value > Maximum)
		{
			UE_LOG(LogTemp, Warning, TEXT("P1 INI: %s 값이 잘못되어 안전값 %.2f를 적용합니다."), Key, SafeValue);
			return SafeValue;
		}
		return Value;
	};
	const float PadCount = ReadNumber(TEXT("TornadoEscapePadCount"), 0.f, 8.f, 0.f);
	TornadoEscapePadCount = FMath::IsNearlyEqual(PadCount, FMath::RoundToFloat(PadCount)) ? FMath::RoundToInt(PadCount) : 0;
	TornadoEscapePadDistance = ReadNumber(TEXT("TornadoEscapePadDistance"), 0.f, 100000.f, 0.f);
	TornadoEscapePadJumpPower = ReadNumber(TEXT("TornadoEscapePadJumpPower"), 0.f, 100000.f, 0.f);
	TornadoKickPower = ReadNumber(TEXT("TornadoKickPower"), 0.f, 100000.f, 0.f);
	TornadoHitInterval = ReadNumber(TEXT("TornadoHitInterval"), 0.1f, 86400.f, 1.f);
	TornadoKickPlayRate = ReadNumber(TEXT("TornadoKickPlayRate"), 0.1f, 5.f, 1.f);
	TornadoKickExtraHeight = ReadNumber(TEXT("TornadoKickExtraHeight"), 0.f, 100000.f, 600.f);
	TornadoLegLengthScale = ReadNumber(TEXT("TornadoLegLengthScale"), 0.1f, 10.f, 1.f);
	FastClearTimeThreshold = ReadNumber(TEXT("FastClearTimeThreshold"), 0.f, 86400.f, 0.f);
	BaseRequiredSubmissionCount = FMath::RoundToInt(ReadNumber(TEXT("BaseRequiredSubmissionCount"), 1.f, 1000.f, 10.f));
	const float ReductionCount = ReadNumber(TEXT("FastClearSubmissionReductionCount"), 0.f, 1000.f, 0.f);
	FastClearSubmissionReductionCount = FMath::IsNearlyEqual(ReductionCount, FMath::RoundToFloat(ReductionCount))
		? FMath::RoundToInt(ReductionCount) : 0;

	TornadoKickChance = ReadNumber(TEXT("TornadoKickChance"), 0.f, 1.f, 1.f);
}

void AGTWBossPhase1Character::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (!GetWorld()->IsGameWorld())
	{
		return;
	}

	LoadBossSettings();
	// INI 배율은 AnimGraph에서 뼈의 위치에만 적용한다. 두께와 발 크기는 늘리지 않는다.
	if (const USkeletalMesh* SkeletalMesh = GetMesh()->GetSkeletalMeshAsset())
	{
		const FReferenceSkeleton& Skeleton = SkeletalMesh->GetRefSkeleton();
		for (const FName Bone : { FName("LeftLeg"), FName("LeftFoot"), FName("RightLeg"), FName("RightFoot") })
		{
			const int32 Index = Skeleton.FindBoneIndex(Bone);
			if (Index != INDEX_NONE)
			{
				TornadoLegReferenceOffsets.Add(Bone, Skeleton.GetRefBonePose()[Index].GetTranslation());
			}
		}
	}
	// 기록이 없거나 조건 미충족이면 기본 제출 목표를 사용한다.
	RequiredSubmissionCount = BaseRequiredSubmissionCount;
	FourthFloorClearTime = -1.0;
	bHasFastClearBenefit = false;
	const UGTWGameInstance* GameInstance = GetGameInstance<UGTWGameInstance>();
	double SokobanTime = -1.0;
	double ShootingTime = -1.0;
	if (!GameInstance
		|| !GameInstance->TryGetMinigameClearTime(EMinigameType::Sokoban, SokobanTime)
		|| !GameInstance->TryGetMinigameClearTime(EMinigameType::Shooting, ShootingTime))
	{
		return;
	}

	// 두 미니게임 기록이 모두 있어야 합산한다. 이동 시간은 포함하지 않는다.
	FourthFloorClearTime = SokobanTime + ShootingTime;
	if (FMath::IsFinite(FourthFloorClearTime) && FourthFloorClearTime >= 0.0
		&& FastClearTimeThreshold > 0.f && FourthFloorClearTime < FastClearTimeThreshold)
	{
		RequiredSubmissionCount = FMath::Max(1, BaseRequiredSubmissionCount - FastClearSubmissionReductionCount);
		bHasFastClearBenefit = RequiredSubmissionCount < BaseRequiredSubmissionCount;
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
	SpawnTornadoEscapePads();
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

FVector AGTWBossPhase1Character::GetTornadoLegOffset(const APawn* PawnOwner, FName BoneName)
{
	// 애니메이션 에디터 미리보기에는 보스 Pawn이 없으므로 기본 포즈를 유지한다.
	const AGTWBossPhase1Character* Boss = Cast<AGTWBossPhase1Character>(PawnOwner);
	const FVector* Offset = Boss ? Boss->TornadoLegReferenceOffsets.Find(BoneName) : nullptr;
	return Offset ? *Offset * ((Boss->TornadoLegLengthScale - 1.f) * Boss->TornadoLegExtensionAlpha) : FVector::ZeroVector;
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
	const float LegBlend = bAirKick ? FMath::Clamp(FMath::Min(Position - StartTime, EndTime - Position) / 0.1f, 0.f, 1.f) : 0.f;
	TornadoLegExtensionAlpha = FMath::SmoothStep(0.f, 1.f, LegBlend);
	const float HeightAlpha = bAirKick ? 1.f : (Section == TEXT("Land") ? 1.f - SmoothAlpha : SmoothAlpha);

	// 좌표 제한은 액터가 담당한다. 시각적인 추가 점프 높이는 메시 오프셋으로 분리한다.
	const float ParentScaleZ = FMath::Max(FMath::Abs(GetActorScale3D().Z), KINDA_SMALL_NUMBER);
	GetMesh()->SetRelativeLocation(TornadoMeshBaseLocation + FVector::UpVector * (FMath::Max(0.f, TornadoKickExtraHeight) * HeightAlpha / ParentScaleZ));
	// 타임라인/Notify 위치는 바꾸지 않고 재생 시계만 조절해 판정 구간이 모션을 따라가게 한다.
	Instance->Montage_SetPlayRate(TornadoKick, bAirKick ? FMath::Clamp(TornadoKickPlayRate, 0.1f, 5.f) : 1.f);
}

void AGTWBossPhase1Character::ResetTornadoKickMotion()
{
	TornadoLegExtensionAlpha = 0.f;
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
	if (bRunning && bTornadoKickPlaying)
	{
		bTornadoDamageActive = true;
		SetTornadoLegCollision(true);
	}
}

void AGTWBossPhase1Character::ApplyTornadoKickContact(AActor* OtherActor)
{
	// 접촉 판정은 BP Box가 담당한다. 기존 플레이어의 넉백/무적 경로만 호출한다.
	if (!bRunning || !bTornadoKickPlaying || !bTornadoDamageActive || GetWorld()->GetTimeSeconds() < NextTornadoHitTime
		|| OtherActor != UGameplayStatics::GetPlayerPawn(this, 0))
	{
		return;
	}
	if (IGTWHitReactionReceiver* Receiver = Cast<IGTWHitReactionReceiver>(OtherActor))
	{
		FVector Direction = (OtherActor->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
		if (Direction.IsNearlyZero())
		{
			Direction = GetActorForwardVector().GetSafeNormal2D();
		}
		Direction.Z = 0.35f;
		// 같은 회전 중에도 재피격 간격이 지나면 다시 접촉을 전달한다.
		NextTornadoHitTime = GetWorld()->GetTimeSeconds() + TornadoHitInterval;
		Receiver->HitKnockbackObject(Direction.GetSafeNormal(), TornadoKickPower);
	}
}

void AGTWBossPhase1Character::EndTornadoKickDamage()
{
	bTornadoDamageActive = false;
	SetTornadoLegCollision(false);
	ClearTornadoEscapePads();
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
	return FindGroundLocation(CandidateLocation, Player, GroundQuery, OutLocation);
}

bool AGTWBossPhase1Character::FindGroundLocation(const FVector& CandidateLocation, APawn* Player, const FCollisionQueryParams& GroundQuery, FVector& OutLocation) const
{
	const FVector PlayerLocation = Player->GetActorLocation();

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

void AGTWBossPhase1Character::SpawnTornadoEscapePads()
{
	ClearTornadoEscapePads();
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!TornadoEscapePadClass || !IsValid(Player))
	{
		return;
	}
	FCollisionQueryParams Query(SCENE_QUERY_STAT(P1EscapePadGround), true, this);
	Query.AddIgnoredActor(Player);
	for (const auto& Actor : SpawnedAttackActors)
	{
		if (Actor.IsValid())
		{
			Query.AddIgnoredActor(Actor.Get());
		}
	}
	for (const auto& Actor : SpawnedHitObjects)
	{
		if (Actor.IsValid())
		{
			Query.AddIgnoredActor(Actor.Get());
		}
	}
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	const int32 Count = FMath::Clamp(TornadoEscapePadCount, 0, 8);
	const float Distance = FMath::Min(FMath::Max(0.f, TornadoEscapePadDistance), FMath::Max(0.f, PlayerRadius));
	const AActor* PadDefaults = TornadoEscapePadClass->GetDefaultObject<AActor>();
	const UBoxComponent* Trigger = Cast<UBoxComponent>(PadDefaults->GetRootComponent());
	const float PadRadius = Trigger ? Trigger->GetScaledBoxExtent().Size2D() : 0.f;
	const ACharacter* Character = Cast<ACharacter>(Player);
	const float PlayerBodyRadius = Character ? Character->GetCapsuleComponent()->GetScaledCapsuleRadius() : 0.f;
	const float MinimumDistance = PadRadius + PlayerBodyRadius + 1.f;
	if (Count == 0 || Distance < MinimumDistance)
	{
		return;
	}
	for (int32 Index = 0; Index < Count; ++Index)
	{
		// 원 안에서 균일하게 랜덤 추출한다. 플레이어 바로 밑·구멍·겹치는 발판은 제외한다.
		for (int32 Attempt = 0; Attempt < 32; ++Attempt)
		{
			const float Angle = FMath::FRandRange(0.f, 2.f * PI);
			const float Radius = FMath::Sqrt(FMath::FRandRange(FMath::Square(MinimumDistance), FMath::Square(Distance)));
			const FVector Candidate = Player->GetActorLocation() + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Radius;
			FVector Location;
			if (!FindGroundLocation(Candidate, Player, Query, Location))
			{
				continue;
			}
			const bool bTooClose = TornadoEscapePads.ContainsByPredicate([&](const TWeakObjectPtr<AActor>& Pad)
			{
				return Pad.IsValid() && FVector::DistSquared2D(Pad->GetActorLocation(), Location) < FMath::Square(PadRadius * 2.f);
			});
			if (bTooClose)
			{
				continue;
			}
			if (AActor* Pad = GetWorld()->SpawnActor<AActor>(TornadoEscapePadClass, Location, FRotator::ZeroRotator, Params))
			{
				TornadoEscapePads.Add(Pad);
				Query.AddIgnoredActor(Pad);
			}
			break;
		}
	}
}

void AGTWBossPhase1Character::ClearTornadoEscapePads()
{
	for (const auto& Pad : TornadoEscapePads)
	{
		if (Pad.IsValid())
		{
			Pad->Destroy();
		}
	}
	TornadoEscapePads.Reset();
}

// 수치는 BP_P1Boss에서 편집하고, C++은 순차 생성과 종료 정리만 담당한다.
void AGTWBossPhase1Character::SpawnAttackActors(APawn* Player)
{
	if (!bRunning || !IsValid(Player) || GetWorldTimerManager().IsTimerActive(AttackSpawnTimer))
	{
		return;
	}
	AttackSpawnTarget = Player;
	RemainingAttackActors = FMath::Clamp(AttackActorCount, 1, 32);
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
	for (const auto& Pad : TornadoEscapePads)
	{
		GroundQuery.AddIgnoredActor(Pad.Get());
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
		if (AGTWMeteorGimmick* Meteor = Cast<AGTWMeteorGimmick>(Attack))
		{
			// 같은 BP_Meteor를 재사용하되, P1에서 생성한 인스턴스만 착지 후 기절시킨다.
			Meteor->GroundedStunDuration = MeteorStunDuration;
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
	for (const auto& Pad : TornadoEscapePads)
	{
		GroundQuery.AddIgnoredActor(Pad.Get());
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
			// BP Construction Script로 메시가 만들어진 뒤 P1 인스턴스에만 적용한다.
			if (HitObjectMaterial)
			{
				TInlineComponentArray<UStaticMeshComponent*> MeshComponents(Object);
				// 원본 BP는 Niagara만 사용한다. P1 인스턴스에 충돌 없는 QR 표시 메시를 붙인다.
				if (MeshComponents.IsEmpty() && HitObjectDisplayMesh)
				{
					UStaticMeshComponent* QRMesh = NewObject<UStaticMeshComponent>(Object, TEXT("P1QRCodeMesh"));
					Object->AddInstanceComponent(QRMesh);
					QRMesh->SetupAttachment(Object->GetRootComponent());
					QRMesh->SetStaticMesh(HitObjectDisplayMesh);
					QRMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					QRMesh->RegisterComponent();
					MeshComponents.Add(QRMesh);
				}
				for (UStaticMeshComponent* MeshComponent : MeshComponents)
				{
					for (int32 SlotIndex = 0; SlotIndex < MeshComponent->GetNumMaterials(); ++SlotIndex)
					{
						MeshComponent->SetMaterial(SlotIndex, HitObjectMaterial);
					}
				}
			}
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
	DisplayedHitCount = FMath::Min(DisplayedHitCount + 1, RequiredSubmissionCount);
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
