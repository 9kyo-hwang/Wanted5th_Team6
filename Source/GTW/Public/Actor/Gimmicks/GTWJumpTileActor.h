// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWJumpTileActor.generated.h"

UCLASS()
class GTW_API AGTWJumpTileActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGTWJumpTileActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PostInitializeComponents() override;

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Interaction)
	float JumpPower;

private:
	UPROPERTY(VisibleAnywhere, Category = JumpTile)
	TObjectPtr<class UStaticMeshComponent> JumpTileMesh;

	UPROPERTY(VisibleAnywhere, Category = JumpTile)
	TObjectPtr<class UBoxComponent> JumpTrigger;
};