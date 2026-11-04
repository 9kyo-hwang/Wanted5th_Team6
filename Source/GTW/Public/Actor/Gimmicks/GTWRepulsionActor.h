// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWRepulsionActor.generated.h"

UCLASS(Blueprintable)
class GTW_API AGTWRepulsionActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGTWRepulsionActor();

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

	void RestoreOriginVolume();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// 반발체 힘
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Interaction)
	float RepulsionPower;

private:
	// 반발체 메시
	UPROPERTY(VisibleAnywhere, Category = JumpTile)
	TObjectPtr<class UStaticMeshComponent> RepulsionMesh;

	// 반발체 트리거
	UPROPERTY(VisibleAnywhere, Category = JumpTile)
	TObjectPtr<class UCapsuleComponent> RepulsionTrigger;

	UPROPERTY()
	FTimerHandle FirstVolumeTimer;
	
	UPROPERTY()
	FTimerHandle SecondVolumeTimer;

	UPROPERTY()
	float VolumeDelta;


};
