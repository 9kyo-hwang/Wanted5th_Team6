// Fill out your copyright notice in the Description page of Project Settings.


#include <Characters/Boss/GTWBossHitObjectSpawner.h>
#include <Characters/Boss/GTWBossHitObject.h>

namespace
{
	static void PrintDebugMessage(const FString& Message, int32 InKey = -1, FColor Color = FColor::Blue)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(InKey, 5.f, Color, Message);
			UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
		}
	}
}

AGTWBossHitObjectSpawner::AGTWBossHitObjectSpawner()
	: RespawnTime(30.f)
	, bIsSpawning(false)
{
	// 에디터에서 Class 타입을 BP로 지정해주자.

	// 얘는 멤버로 들고 있을 필요가 없어서 바로 Set
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot")));
}

void AGTWBossHitObjectSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 레벨이 내려가면 스포너도 지워지므로, HitObject도 같이 없앨 수 있도록 호출 필요
	StopSpawning();

	Super::EndPlay(EndPlayReason);
}

bool AGTWBossHitObjectSpawner::StartSpawning(AActor* InTarget)
{
	if (bIsSpawning)
	{
		return true;
	}

	if (!IsValid(HitObjectClass) || !IsValid(InTarget) || RespawnTime <= 0.f)
	{
		return false;
	}

	TargetActorRef = InTarget;
	bIsSpawning = true;

	// 생성 타이머 ON
	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&ThisClass::SpawnHitObject,
		RespawnTime,
		false
	);
	
	return true;
}

void AGTWBossHitObjectSpawner::StopSpawning()
{
	bIsSpawning = false;
	GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
	
	// 단순 null 체크 X, 
	// Object가 Destroy되면 GC 수집 대상이 되면서 Invalid
	// 즉 다시 소환해야 하는 상태가 되는 것
	if (IsValid(HitObjectInstance.Get()))
	{
		HitObjectInstance->Cancel();
	}

	HitObjectInstance = nullptr;
	TargetActorRef = nullptr;
}

void AGTWBossHitObjectSpawner::SpawnHitObject()
{
	if (!bIsSpawning)
	{
		return;
	}

	if (!TargetActorRef.IsValid())
	{
		PrintDebugMessage(TEXT("Target Actor is NOT Valid"));
		StopSpawning();
		return;
	}

	// 단순 null 체크 X, 
	// Object가 Destroy되면 GC 수집 대상이 되면서 Invalid
	// 즉 다시 소환해야 하는 상태가 되는 것
	if (IsValid(HitObjectInstance.Get()))
	{
		PrintDebugMessage(TEXT("HitObject is ALREADY Instanced"));
		StopSpawning();
		return;
	}

	// @TEMP: Spawner 위치 그대로 생성
	if (AGTWBossHitObject* HitObject = GetWorld()->SpawnActor<AGTWBossHitObject>(HitObjectClass, GetActorLocation(), GetActorRotation()))
	{
		HitObjectInstance = HitObject;

		// @TODO: HitObject에 접촉 대기 종료 델리게이트 연결해줘야 함
		HitObjectInstance->OnContactEnded.BindUObject(this, &ThisClass::HandleContactEnded);

		if (!HitObjectInstance->Activate(TargetActorRef.Get()))
		{
			PrintDebugMessage(TEXT("[FAILED] HitObject Activate"));
			StopSpawning();
		}

		return;
	}

	PrintDebugMessage(TEXT("[FAILED] HitObject Spawn"));

	// SpawnActor 자체가 안되니 정리해버리자.
	StopSpawning();
}

// HitObject에서 접촉 완료되어 HitObject를 재생성해야할 때
// HitObject의 Delegate를 통해 호출될 Callback
void AGTWBossHitObjectSpawner::HandleContactEnded()
{
	if (!bIsSpawning)
	{
		PrintDebugMessage(TEXT("[HandleContactEnded] 재생성 해야하는 데 생성 Stop 됨(bIsSpawning == false)"));
		return;
	}

	// 재생성해야 하니 Timer 다시 ON
	GetWorldTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&ThisClass::SpawnHitObject,
		RespawnTime,
		false
	);
}
