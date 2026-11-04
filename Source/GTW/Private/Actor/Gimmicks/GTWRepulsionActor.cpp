// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmicks/GTWRepulsionActor.h"

#include <Interfaces/GTWInteractionGimmicksInterface.h>
#include <Components/CapsuleComponent.h>
#include <Components/StaticMeshComponent.h>


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
	RepulsionTrigger->SetCollisionProfileName(TEXT("GTWCollision"));

	// 트리거
	RepulsionTrigger->SetCapsuleSize(70.0f, 70.0f);
	RepulsionTrigger->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f));

	// 메시
	RepulsionMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	RepulsionMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 1.0f));

	// 콜리전
	RepulsionMesh->SetCollisionProfileName(TEXT("GTWCollision"));


	// 임시 에셋
	static ConstructorHelpers::FObjectFinder<UStaticMesh> RepulsionRef(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder")
	);
	if (RepulsionRef.Succeeded())
	{
		RepulsionMesh->SetStaticMesh(RepulsionRef.Object);
	}

	// 반발체 힘
	RepulsionPower = 10.0f;
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
		OverlappingPawn->InteractRepulsion(PulseDirecion, RepulsionPower);
	}
	// 반발체
	FVector MeshScale = RepulsionMesh->GetRelativeScale3D();

	// 부피 변화량
	MeshScale = MeshScale * (VolumeDelta * VolumeDelta);

	// 부피 변화
	RepulsionMesh->SetRelativeScale3D(MeshScale);

	// 복원 시간 
	float FirstRestoreTime = 0.1f;
	float SecondRestoreTime = 0.3f;

	GetWorldTimerManager().SetTimer(
		FirstVolumeTimer,
		this,
		&AGTWRepulsionActor::RestoreOriginVolume,
		FirstRestoreTime,
		false
	);
	GetWorldTimerManager().SetTimer(
		SecondVolumeTimer,
		this,
		&AGTWRepulsionActor::RestoreOriginVolume,
		SecondRestoreTime,
		false
	);

}

void AGTWRepulsionActor::RestoreOriginVolume()
{
	FVector MeshScale = RepulsionMesh->GetRelativeScale3D();

	// 부피 변환
	MeshScale = MeshScale / VolumeDelta;

	RepulsionMesh->SetRelativeScale3D(MeshScale);
}

// Called every frame
void AGTWRepulsionActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

