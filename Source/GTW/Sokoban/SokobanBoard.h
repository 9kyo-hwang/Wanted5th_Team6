// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SokobanBoard.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class AGTWSokobanBox;

UCLASS()
class GTW_API ASokobanBoard : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASokobanBoard();
	bool TryPushBox(
		AGTWSokobanBox* Box,
		const FIntPoint& Direction
	);
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Sokoban|WallMesh")
	TObjectPtr<UStaticMesh> WallStaticMesh;

	UPROPERTY(EditAnywhere, Category = "Sokoban|TileMesh")
	TObjectPtr<UStaticMesh> FloorStaticMesh;

	// Goal 블루프린트
	UPROPERTY(EditAnywhere, Category = "Sokoban|Goal")
	TSubclassOf<AActor> ResumeGoalTileClass;

	UPROPERTY(EditAnywhere, Category = "Sokoban|Goal")
	TSubclassOf<AActor> IntroductionGoalTileClass;

	UPROPERTY(EditAnywhere, Category = "Sokoban|Goal")
	TSubclassOf<AActor> PortfoliGoalTileClass;

	// 상자 블루프린트
	UPROPERTY(EditAnywhere, Category = "Sokoban|Box")
	TSubclassOf<AGTWSokobanBox> RedBoxClass;

	UPROPERTY(EditAnywhere, Category = "Sokoban|Box")
	TSubclassOf<AGTWSokobanBox> GreenBoxClass;

	UPROPERTY(EditAnywhere, Category = "Sokoban|Box")
	TSubclassOf<AGTWSokobanBox> BlueBoxClass;

	UPROPERTY(EditAnywhere, Category = "Sokoban|Tilebetween")
	float TileSize = 100.0f;

	// 함수
private:
	bool LoadMapFileData();

	// 맵 데이터를 2차원  배열로 변환
	void InitializeMap();
	void CreateTileMesh(UStaticMesh* MeshAsset, const FVector& Location,
		UMaterialInterface* Material = nullptr);

	AGTWSokobanBox* SpawnBox(TSubclassOf<AGTWSokobanBox> BoxClass, const FVector& TileLocation);

	UPROPERTY(Transient)
	TArray<TObjectPtr<AGTWSokobanBox>> SpawnedBoxes;

	FIntPoint WorldToGrid(const FVector& WorldLocation) const;

	// 변수
private:
	int32 Rows; // 행
	int32 Colums; // 열

	// 2차원 맵 데이터 [MapRows]로 일단 사용할건데 나중에 문제 생기면 교체용으로 일단 저장
	TArray<TArray<TCHAR>> MapGrid;

	// 맵 데이터 - 파일에서 string값으로 불러오는변수
	FString MapDataText;

	// 2차원배열로 실제 사용중인 mapdata 
	TArray<FString> MapRows;

};
