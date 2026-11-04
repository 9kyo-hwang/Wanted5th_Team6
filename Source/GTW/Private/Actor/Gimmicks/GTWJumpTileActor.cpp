// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Gimmicks/GTWJumpTileActor.h"

#include <Interfaces/GTWInteractionGimmicksInterface.h>
#include <Components/BoxComponent.h>
#include <Components/StaticMeshComponent.h>


// Sets default values
AGTWJumpTileActor::AGTWJumpTileActor()
{
	// 컴포넌트 생성
	JumpTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	JumpTileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	// 계층 설정
	RootComponent = JumpTrigger;
	JumpTileMesh->SetupAttachment(JumpTrigger);

	// 박스 컴포넌트에 콜리전 프로필 설정.
	JumpTrigger->SetCollisionProfileName(TEXT("GTWCollision"));

	// 트리거
	JumpTrigger->SetBoxExtent(FVector(130.0f, 130.0f, 20.0f));
	JumpTrigger->SetRelativeLocation(FVector(0.0f, 0.0f, 300.0f));

	// 메시
	JumpTileMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -10.0f));
	JumpTileMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 0.05f));

	// 콜리전
	JumpTileMesh->SetCollisionProfileName(TEXT("GTWCollision"));


	// 임시 에셋
	static ConstructorHelpers::FObjectFinder<UStaticMesh> JumpTileRef(
		TEXT("/Engine/EngineMeshes/Cube.Cube")
	);
	if (JumpTileRef.Succeeded())
	{
		JumpTileMesh->SetStaticMesh(JumpTileRef.Object);
	}

}
// Called when the game starts or when spawned
void AGTWJumpTileActor::BeginPlay()
{
	Super::BeginPlay();
}

void AGTWJumpTileActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 박스 컴포넌트의 BeginOverlap 델리게이트에 함수 등록
	JumpTrigger->OnComponentBeginOverlap.AddDynamic(this, &AGTWJumpTileActor::OnOverlapBegin);

	// 점프 힘 
	JumpPower = 2000.0f;
}

void AGTWJumpTileActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 인터페이스 업캐스팅
	IGTWInteractionGimmicksInterface* OverlappingPawn =
		Cast<IGTWInteractionGimmicksInterface>(OtherActor);

	if (OverlappingPawn)
	{
		OverlappingPawn->ApplyJumpImpulse(JumpPower);
	}
}


// Called every frame
void AGTWJumpTileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

