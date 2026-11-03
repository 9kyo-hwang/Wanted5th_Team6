// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GTWRoadblockGimmick.generated.h"

/*
* 1. Player 진입 감지
* 2. 목표 위치 예고 표시
* 3. 일정 시간 후 길막 액터 발사
* 4. 액터 이동(임시로 직선 이동)
* 5. 도달 시 Collision Enable
* 6. EndPlay에서 정리
*/

UENUM(BlueprintType)
enum class ERoadblockState : uint8
{
	Waiting,
	Warning,
	Flying,
	Blocked
};

UCLASS()
class GTW_API AGTWRoadblockGimmick : public AActor
{
	GENERATED_BODY()
	
public:	
	AGTWRoadblockGimmick();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION()
	void OnPlayerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);	// 플레이어 진입
	void Warning();			// 경고 마커 표시 및 발사 준비
	void Launch();			// 발사
	void Finish();			// 도착

private:
	/*
	* 필요한거
	* - Player 감지용 트리거 박스
	* - 발사대 메시
	* - 발사 및 도착 지점
	* - 충돌체
	*/

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneComponent> Root;

	// 플레이어 탐지용 트리거 박스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Boss|Gimmick", meta=(AllowPrivateAccess = "true"))
	TObjectPtr<class UBoxComponent> TriggerBox;

	// 발사대
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> LauncherMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USceneComponent> LaunchPoint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UArrowComponent> TargetPoint;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> WarningMarker;	// 예고 지점 표시할 표식.

	// 길막 액터
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USphereComponent> BlockCollision;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> BlockMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	float WarningDuration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Boss|Gimmick", meta = (AllowPrivateAccess = "true"))
	float FlightDuration;

	ERoadblockState CurrentState;
	FTimerHandle WarningTimerHandle;
	float FlightElapsedTime;
	FVector FlightStartLocation;
	FVector FlightTargetLocation;
};
