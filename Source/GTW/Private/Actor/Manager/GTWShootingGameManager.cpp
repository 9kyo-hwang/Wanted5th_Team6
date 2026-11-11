// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Manager/GTWShootingGameManager.h"
#include <Kismet/GameplayStatics.h>
#include <Blueprint/UserWidget.h>
#include <GameFramework/PlayerController.h>
#include <GameFramework/Character.h>
#include <GTWGameInstance.h>

// Sets default values
AGTWShootingGameManager::AGTWShootingGameManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AGTWShootingGameManager::BeginPlay()
{
	Super::BeginPlay();
	Player = UGameplayStatics::GetPlayerController(this, 0);
	GameInstance = GetGameInstance<UGTWGameInstance>();
	if (IsValid(StartLocationActor))
	{
		StartPlayerLocation = StartLocationActor->GetActorLocation();
	}

	if (IsValid(ShootingBossActor))
	{
		StartBossLocation = ShootingBossActor->GetActorLocation();
	}

	if (IsValid(ForwardMovingTriggerActor))
	{
		StartForwardMovingLocation = ForwardMovingTriggerActor->GetActorLocation();
	}

	UGTWGameInstance* GameInst = Cast<UGTWGameInstance>(GetGameInstance());

	if (!IsValid(GameInst))
	{
		return;
	}

	bIsShootingClear = GameInst->IsLevelCleared(ELevelClear::ShootingGameClear);
}

void AGTWShootingGameManager::ShowElapsedWidget()
{
	ElapsedWidget = CreateWidget<UUserWidget>(
		Player,
		ElapsedWidgetClass
	);

	if (IsValid(ElapsedWidget))
	{
		ElapsedWidget->AddToViewport();
		ElapsedWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void AGTWShootingGameManager::ShowClearViewWidget()
{
	ClearViewWidget = CreateWidget<UUserWidget>(
		Player,
		ClearViewWidgetClass
	);

	if (IsValid(ClearViewWidget))
	{
		ClearViewWidget->AddToViewport();
		ClearViewWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

// Called every frame
void AGTWShootingGameManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsGameplayTimerRunning)
	{
		ElapsedTime += DeltaTime;
	}
}

void AGTWShootingGameManager::ShootingGameStart()
{
	bIsGameplayTimerRunning = true;
	ShowElapsedWidget();
	Player->SetIgnoreMoveInput(false);
	FRotator StartRotator = FRotator(Player->TargetViewRotation.Pitch, Player->TargetViewRotation.Yaw,
		Player->TargetViewRotation.Roll);
	Player->GetCharacter()->TeleportTo(StartPlayerLocation, StartRotator);
}

void AGTWShootingGameManager::Reset()
{
	Player->GetCharacter()->TeleportTo(StartPlayerLocation, Player->TargetViewRotation);
	ShootingBossActor->TeleportTo(StartBossLocation, ShootingBossActor->GetActorRotation());
	ForwardMovingTriggerActor->TeleportTo(StartForwardMovingLocation,
		ForwardMovingTriggerActor->GetActorRotation());
}

void AGTWShootingGameManager::GameClear()
{
	bIsGameplayTimerRunning = false;
	bIsShootingClear = true;
	GameInstance->SetMinigameClearTime(EMinigameType::Shooting, ElapsedTime);
	ShowClearViewWidget();
	Player->GetCharacter()->TeleportTo(ShootingBoxTriggerActor->GetActorLocation(),
		Player->GetCharacter()->GetActorRotation());

	if (IsValid(ElapsedWidget))
	{
		ElapsedWidget->RemoveFromParent();
		ElapsedWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}