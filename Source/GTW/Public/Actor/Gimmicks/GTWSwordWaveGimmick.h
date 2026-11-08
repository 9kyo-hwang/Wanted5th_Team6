// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWSwordWaveGimmick.generated.h"

UCLASS()
class GTW_API AGTWSwordWaveGimmick : public AActor
{
	GENERATED_BODY()
	
public:	
	AGTWSwordWaveGimmick();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION()
	void OnPlayerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	void OnEndWarning();

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|SwordWave", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneComponent> Root;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|SwordWave", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBoxComponent> WaveCollision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|SwordWave", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UParticleSystemComponent> WaveFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|SwordWave", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|SwordWave", meta = (AllowPrivateAccess = "true"))
	float WarningDuration;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|SwordWave", meta = (AllowPrivateAccess = "true"))
	float MovementSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick|SwordWave", meta = (AllowPrivateAccess = "true"))
	float LifeSpan;

	FTimerHandle WarningTimerHandle;
};
