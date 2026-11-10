// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "GTWPlayerAudioComponent.generated.h"

/**
 * 
 */
UCLASS()
class GTW_API UGTWPlayerAudioComponent : public UAudioComponent
{
	GENERATED_BODY()
	
public:
	UGTWPlayerAudioComponent();

    virtual void InitializeComponent() override;

public:
    // 사운드 실행 함수
    void PlaySkateStart();
    void PlaySkateLoop();
    void PlayHitSound();
    void PlayStrongHitSound();
    void PlayImpactVoiceSound();
    // 루프 사운드 종료 함수
    void EndSkateLoop();


protected:
    // 오너
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Audio)
    TObjectPtr<class AGTWPlayerCharacter> GTWPlayerCharacter;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<class USoundBase> WeakHitSound;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> StrongHitSound;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> HitSound;
    
    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> SkateStartSound;
    
    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> SkateLoopSound;
    
    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> SkateEndSound;
    
    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> HardImpactVoiceSound;
};
