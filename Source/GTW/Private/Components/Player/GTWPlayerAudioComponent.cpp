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

