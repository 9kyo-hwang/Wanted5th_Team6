// Fill out your copyright notice in the Description page of Project Settings.

#include "../../Public/Characters/GTWPlayerCharacter.h"
#include "../../Public/Components/GTWSkateMovementComponent.h"

// Sets default values
AGTWPlayerCharacter::AGTWPlayerCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// 컴포넌트 획득
	SkateCharacterMovementComponent = Cast<UGTWSkateMovementComponent>(GetCharacterMovement());
}

// Called when the game starts or when spawned
void AGTWPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGTWPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AGTWPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

} 

FCollisionQueryParams AGTWPlayerCharacter::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams Params;

	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}

 