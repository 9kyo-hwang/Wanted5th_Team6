#include "Characters/Boss/GTWBoss1.h"
#include "Engine/TriggerBox.h"
#include "Engine/World.h"
#include "Controllers/GTWBoss1PAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

/*
* 설정은 BP에서 관리하고, 이 파일은 P1 보스의 실제 동작만 처리한다.
*/
void AGTWBoss1::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    // 보스 이동 범위 제한
    EnforceArenaBounds();
    DrawArenaDebug();
}

void AGTWBoss1::EnforceArenaBounds()
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
FVector AGTWBoss1::GetMovementExtent() const
{
    const FTransform Transform = GetMovementBounds()->GetComponentTransform();
    const FVector Extent = GetMovementBounds()->GetUnscaledBoxExtent();
    const FVector Scale = Transform.GetScale3D().GetAbs();
    const float Radius = GetCapsuleComponent()->GetScaledCapsuleRadius();

    const float X = FMath::Max(0.f, Extent.X - Radius / FMath::Max(Scale.X, KINDA_SMALL_NUMBER));
    const float Y = FMath::Max(0.f, Extent.Y - Radius / FMath::Max(Scale.Y, KINDA_SMALL_NUMBER));

    return FVector(X, Y, Extent.Z);
}

/* 주어진 위치를 사각형 안의 추격 목표로 변환 */
FVector AGTWBoss1::ConstrainPosition(const FVector& Position) const
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

FVector AGTWBoss1::GetChaseLocation(const FVector& PlayerPosition) const
{
    const FTransform Transform = GetMovementBounds()->GetComponentTransform();
    const FVector Extent = GetMovementExtent();
    FVector Local = Transform.InverseTransformPosition(PlayerPosition);

    // 플레이어가 사각형 밖에 있어도 방향을 유지하여 모서리 추격이 한 점에 멈추지 않게 한다.
    float Factor = 1.f;
    if (FMath::Abs(Local.X) > Extent.X)
    {
        Factor = FMath::Min(Factor, Extent.X / FMath::Abs(Local.X));
    }
    if (FMath::Abs(Local.Y) > Extent.Y)
    {
        Factor = FMath::Min(Factor, Extent.Y / FMath::Abs(Local.Y));
    }
    Local.X *= Factor;
    Local.Y *= Factor;
    FVector Result = Transform.TransformPosition(Local);
    Result.Z = BossHeight;

    return Result;
}

/* 전투 중에만 기본 이동을 중지하고 종료 시 복구한다. 회전 옵션은 BP에서 설정한다. */
void AGTWBoss1::PrepareEncounter()
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    PreviousMovementMode = Movement->MovementMode;
    PreviousCustomMode = Movement->CustomMovementMode;

    Movement->StopMovementImmediately();
    Movement->DisableMovement();
}

void AGTWBoss1::UpdatePursuit(APawn* TargetPlayer, const FVector& ChaseLocation, float DeltaSeconds)
{
    if (!IsValid(MovementBoundsActor) || !IsEncounterRunning() || !IsValid(TargetPlayer))
    {
        return;
    }

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

void AGTWBoss1::PlayDropObstacles(APawn* Player)
{
    // 공격 가능 여부는 보스 한 곳에서 판단한다. Task는 대상 전달만 담당한다.
    if (!bRunning || !IsValid(MovementBoundsActor) || !IsValid(Player) || IsPreparingAttack())
    {
        return;
    }
    // P2와 같은 Boss 함수 -> 공통 몽타주 재생 흐름. P1은 모션 정상 종료 후 실제 공격을 실행한다.
    if (!PlayAttackAnim(DropAttack, Player))
    {
        SpawnAttackActors(Player);
    }
}

// 기믹 BP와 공격 수치는 BP_P1Boss에서 편집한다.
TArray<TSubclassOf<AActor>> AGTWBoss1::GetAttackClasses() const
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
bool AGTWBoss1::FindAttackSpawnLocation(APawn* Player, const FCollisionQueryParams& GroundQuery, FVector& OutLocation) const
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

void AGTWBoss1::SpawnAttackActors(APawn* Player)
{
    if (!IsValid(MovementBoundsActor) || !IsEncounterRunning() || !IsValid(Player))
    {
        return;
    }

    const auto Classes = GetAttackClasses();
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

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const float Scale = FMath::Max(0.1f, GroundAttackScale);
    const int32 DesiredCount = FMath::Clamp(AttackActorCount, 1, 32);
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

bool AGTWBoss1::IsPreparingAttack() const
{
    return PreparedMontage != nullptr;
}

bool AGTWBoss1::PlayAttackAnim(UAnimMontage* Montage, APawn* Player)
{
    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!Montage || !AnimInstance)
    {
        return false;
    }

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

void AGTWBoss1::OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage != PreparedMontage)
    {
        return;
    }

    APawn* Player = PreparedTarget.Get();
    PreparedTarget.Reset();
    PreparedMontage = nullptr;

    if (!bInterrupted)
    {
        SpawnAttackActors(Player);
    }
}

void AGTWBoss1::CancelPreparedAttack()
{
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

void AGTWBoss1::StopEncounter()
{
    if (!bRunning)
    {
        return;
    }
    // 늦게 도착한 몽타주 종료 콜백도 공격할 수 없도록 먼저 종료 상태로 바꾼다.
    bRunning = false;
    if (auto* AI = Cast<AGTWBoss1PAIController>(GetController()))
    {
        AI->StopAI();
    }
    CancelPreparedAttack();
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

void AGTWBoss1::EndPlay(const EEndPlayReason::Type Reason)
{
    StopEncounter();
    Super::EndPlay(Reason);
}

/* 전투 시작 */
void AGTWBoss1::StartEncounter()
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

    auto* AI = Cast<AGTWBoss1PAIController>(GetController());
    if (!AI)
    {
        return;
    }

    PrepareEncounter();
    bRunning = true;
    if (!AI->RunAI())
    {
        StopEncounter();
    }
}

void AGTWBoss1::DrawArenaDebug() const
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

UBoxComponent* AGTWBoss1::GetMovementBounds() const
{
    return IsValid(MovementBoundsActor) ? Cast<UBoxComponent>(MovementBoundsActor->GetCollisionComponent()) : nullptr;
}
