// Fill out your copyright notice in the Description page of Project Settings.


#include <Sokoban/GTWSokobanBoard.h>
#include <Sokoban/GTWSokobanBox.h>

// Sets default values
AGTWSokobanBox::AGTWSokobanBox()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGTWSokobanBox::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGTWSokobanBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    if (!bIsMoving)
    {
        return;
    }

    ElapsedTime += DeltaTime;

    const float Alpha = FMath::Clamp(
        ElapsedTime / MoveDuration,
        0.f,
        1.f
    );

    // 서서히 출발하고 도착할 때 감속
    const float SmoothAlpha =
        FMath::InterpEaseInOut(0.f, 1.f, Alpha, 2.f);

    const FVector NewLocation = FMath::Lerp(
        StartLocation,
        TargetLocation,
        SmoothAlpha
    );

    SetActorLocation(NewLocation);

    if (Alpha >= 1.f)
    {
        SetActorLocation(TargetLocation);
        bIsMoving = false;
    }
}

bool AGTWSokobanBox::StartMove(const FVector& Destination)
{
    if (bIsMoving || MoveDuration <= 0.f)
    {
        return false;
    }

    StartLocation = GetActorLocation();
    TargetLocation = Destination;

    // 상자 높이는 기존 Z 값으로 유지
    TargetLocation.Z = StartLocation.Z;

    ElapsedTime = 0.f;
    bIsMoving = true;

    return true;
}

void AGTWSokobanBox::RequestPush(AActor* PushingActor)
{
    if (!IsValid(PushingActor) || bIsMoving)
    {
        return;
    }

    // 상자를 기준으로 플레이어가 어느 방향에 있는지 계산
    const FVector BoxLocation = GetActorLocation();
    const FVector PlayerLocation = PushingActor->GetActorLocation();

    const FVector Distance = BoxLocation - PlayerLocation;

    FIntPoint PushDirection = FIntPoint::ZeroValue;
    const float AbsX = FMath::Abs(Distance.X);
    const float AbsY = FMath::Abs(Distance.Y);

    // 정확히 모서리 닿으면 이동불가 처리
    if (FMath::IsNearlyEqual(AbsX, AbsY, 1.0f))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("45 Degree Push Blocked"));
        return;
    }

    if (AbsX > AbsY)
    {
        // X축 방향
        PushDirection.X = Distance.X >= 0.f ? 1 : -1;
    }
    else
    {
        // Y축 방향
        PushDirection.Y = Distance.Y >= 0.f ? 1 : -1;
    }

    AGTWSokobanBoard* Board = Cast<AGTWSokobanBoard>(GetOwner());

    if (!Board)
    {
        UE_LOG(LogTemp, Error, TEXT("Board Owner Not Found!"));
        return;
    }

    Board->TryPushBox(this, PushDirection);
}

