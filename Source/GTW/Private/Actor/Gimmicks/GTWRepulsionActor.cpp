// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmicks/GTWRepulsionActor.h"

#include <Interfaces/GTWInteractionGimmicksInterface.h>
#include <Components/CapsuleComponent.h>
#include <Components/StaticMeshComponent.h>
#include "UObject/ConstructorHelpers.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"

#include <cassert>


// Sets default values
AGTWRepulsionActor::AGTWRepulsionActor()
{
	// 컴포넌트 생성
	RepulsionTrigger = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Trigger"));
	RepulsionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	// 계층 설정
	RootComponent = RepulsionTrigger;
	RepulsionMesh->SetupAttachment(RepulsionTrigger);

	// 박스 컴포넌트에 콜리전 프로필 설정.
	RepulsionTrigger->SetCollisionProfileName(TEXT("OverlapOnlyPawn"));

	// 트리거
	RepulsionTrigger->SetCapsuleSize(55.0f, 70.0f);
	RepulsionTrigger->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

	// 메시
	RepulsionMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	RepulsionMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));

	// 콜리전
	RepulsionMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));


	// 임시 에셋
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RepulsionRef(
		TEXT("/Game/Assets/GImmicks/2F/Meshes/SM_GTW_RepulsionBumper.SM_GTW_RepulsionBumper")
	);
	if (RepulsionRef.Succeeded())
	{
		RepulsionMesh->SetStaticMesh(RepulsionRef.Object);
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> SoundAsset(
		TEXT("/Game/Assets/Sounds/Repulse.Repulse")
	);

	if (SoundAsset.Succeeded())
	{
		RepulsionSound = SoundAsset.Object;
	}


	// 반발체 힘
	RepulsionPower = 1000.0f;
}

// Called when the game starts or when spawned
void AGTWRepulsionActor::BeginPlay()
{
	Super::BeginPlay();

	// 부피 변환 값 설정
	VolumeDelta = 1.2f;
}

void AGTWRepulsionActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 캡슐 컴포넌트의 BeginOverlap 델리게이트에 함수 등록
	RepulsionTrigger->OnComponentBeginOverlap.AddDynamic(this, &AGTWRepulsionActor::OnOverlapBegin);

}

void AGTWRepulsionActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == this)
	{
		return;
	}

	FVector PulseDirecion = OtherActor->GetActorLocation() - GetActorLocation();
	PulseDirecion.Z = 0;
	PulseDirecion.Normalize();

	// 인터페이스 업캐스팅
	IGTWInteractionGimmicksInterface* OverlappingPawn =
		Cast<IGTWInteractionGimmicksInterface>(OtherActor);

	if (OverlappingPawn)
	{
		// 액터 밀어내기 
		OverlappingPawn->ApplyRepulsion(PulseDirecion, RepulsionPower);

		// 사운드 재생
		if (RepulsionSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				RepulsionSound,
				GetActorLocation()
			);
		}
	}

	// 펌핑 시간 && 복원 시간 
	float ExpandedTime = 0.05f;
	float FirstRestoreTime = 0.2f;
	float SecondRestoreTime = 0.3f;

	GetWorldTimerManager().SetTimer(
		ExpandedVolumeTimer,
		this,
		&AGTWRepulsionActor::ExpandOnRepulsion,
		ExpandedTime,
		false
	);
	GetWorldTimerManager().SetTimer(
		FirstRestoreVolumeTimer,
		this,
		&AGTWRepulsionActor::RestoreOriginVolume,
		FirstRestoreTime,
		false
	);
	GetWorldTimerManager().SetTimer(
		SecondRestoreVolumeTimer,
		this,
		&AGTWRepulsionActor::RestoreOriginVolume,
		SecondRestoreTime,
		false
	);

	float ForcedRestoreTime = 1.0f;
	GetWorldTimerManager().SetTimer(
		ForceRestoreVolumeTimer,
		this,
		&AGTWRepulsionActor::ForceOriginalVolume,
		ForcedRestoreTime,
		false
	);

}

void AGTWRepulsionActor::ExpandOnRepulsion()
{
	FVector MeshScale = RepulsionMesh->GetRelativeScale3D();

	// 부피 변환 1.44배 
	MeshScale = MeshScale * VolumeDelta * VolumeDelta;

	RepulsionMesh->SetRelativeScale3D(MeshScale);
}

void AGTWRepulsionActor::RestoreOriginVolume()
{
	FVector MeshScale = RepulsionMesh->GetRelativeScale3D();

	// 부피 변환 / 1.2 배
	MeshScale = MeshScale / VolumeDelta;

	RepulsionMesh->SetRelativeScale3D(MeshScale);
}

void AGTWRepulsionActor::ForceOriginalVolume()
{
	RepulsionMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
}

// Called every frame
void AGTWRepulsionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

