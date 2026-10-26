// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWSokobanBox.generated.h"

UENUM(BlueprintType)
enum class BoxType : uint8
{
	Resume,
	Introduction,
	Portfolio
};

UCLASS()
class GTW_API AGTWSokobanBox : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGTWSokobanBox();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Board가 이동 가능하다고 판단한 후 호출
	bool StartMove(const FVector& Destination);

	UFUNCTION(BlueprintCallable, Category = "Sokoban|Push")
	void RequestPush(AActor* PushingActor);

	bool IsMoving() const { return bIsMoving; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Sokoban|Move")
	float MoveDuration = 0.5f;
public:	
	FVector StartLocation;
	FVector TargetLocation;

	float ElapsedTime = 0.f;
	bool bIsMoving = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,Category = "BoxTypeEnum")
	BoxType EBoxType;	
};
