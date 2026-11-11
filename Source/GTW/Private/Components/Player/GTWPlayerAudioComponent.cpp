// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Player/GTWPlayerAudioComponent.h"
#include <Characters/Player/GTWPlayerCharacter.h>


#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"

UGTWPlayerAudioComponent::UGTWPlayerAudioComponent()
{
    bAutoActivate = false;
    bWantsInitializeComponent = true;

    static ConstructorHelpers::FObjectFinder<USoundBase> WeakHitSoundAsset(
        TEXT("/Game/Assets/Sounds/Weak.Weak")
    );
    if (WeakHitSoundAsset.Succeeded())
    {
        WeakHitSound = WeakHitSoundAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> StrongHitSoundAsset(
        TEXT("/Game/Assets/Sounds/Strong.Strong")
    );
    if (StrongHitSoundAsset.Succeeded())
    {
        StrongHitSound = StrongHitSoundAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> HitSoundAsset(
        TEXT("/Game/Assets/Sounds/Hit.Hit")
    );
    if (HitSoundAsset.Succeeded())
    {
        HitSound = HitSoundAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> SkateStartSoundAsset(
        TEXT("/Game/Assets/Sounds/skate_start.skate_start")
    );
    if (SkateStartSoundAsset.Succeeded())
    {
        SkateStartSound = SkateStartSoundAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> SkateLoopSoundAsset(
        TEXT("/Game/Assets/Sounds/skate_loop.skate_loop")
    );
    if (SkateLoopSoundAsset.Succeeded())
    {
        SkateLoopSound = SkateLoopSoundAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> SkateEndSoundAsset(
        TEXT("/Game/Assets/Sounds/skate_stop.skate_stop")
    );
    if (SkateEndSoundAsset.Succeeded())
    {
        SkateEndSound = SkateEndSoundAsset.Object;
    }

    static ConstructorHelpers::FObjectFinder<USoundBase> HardImpactVoiceSoundAsset(
        TEXT("/Game/Assets/Sounds/HardImpactVoice.HardImpactVoice")
    );
    if (HardImpactVoiceSoundAsset.Succeeded())
    {
        HardImpactVoiceSound = HardImpactVoiceSoundAsset.Object;
    }
}

void UGTWPlayerAudioComponent::InitializeComponent()
{
    Super::InitializeComponent();

	GTWPlayerCharacter = Cast<AGTWPlayerCharacter>(GetOwner());
}

void UGTWPlayerAudioComponent::PlaySkateStart()
{
    if (!GTWPlayerCharacter)
    {
        return;
    }

    if (!SkateStartSound)
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(
        this,
        SkateStartSound,
        GetOwner()->GetActorLocation()
    );
}

void UGTWPlayerAudioComponent::PlaySkateLoop()
{
    if (!GTWPlayerCharacter)
    {
        return;
    }

    if (!SkateLoopSound)
    {
        return;
    }

    // 같은 루프가 재생 중이면 유지
    if (Sound == SkateLoopSound && IsPlaying())
    {
        return;
    }

    SetSound(SkateLoopSound);
    Play();
}

void UGTWPlayerAudioComponent::PlayHitSound()
{
    if (!GTWPlayerCharacter)
    {
        return;
    }

    if (!HitSound)
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(
        this,
        HitSound,
        GetOwner()->GetActorLocation()
    );
}

void UGTWPlayerAudioComponent::PlayStrongHitSound()
{
    if (!GTWPlayerCharacter)
    {
        return;
    }

    if (!StrongHitSound)
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(
        this,
        StrongHitSound,
        GetOwner()->GetActorLocation()
    );
}

void UGTWPlayerAudioComponent::PlayImpactVoiceSound()
{
    if (!GTWPlayerCharacter)
    {
        return;
    }

    if (!HardImpactVoiceSound)
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(
        this,
        HardImpactVoiceSound,
        GetOwner()->GetActorLocation()
    );
}

void UGTWPlayerAudioComponent::EndSkateLoop()
{
    Stop();
}

