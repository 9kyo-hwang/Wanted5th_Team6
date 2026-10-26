// Fill out your copyright notice in the Description page of Project Settings.


#include "SokobanBoard.h"
#include "GTWSokobanBox.h"

#include <Misc/FileHelper.h>
#include <Misc/Paths.h>
#include <Engine/StaticMesh.h>
#include <Components/StaticMeshComponent.h>

// Sets default values
ASokobanBoard::ASokobanBoard()
	: Rows(0), Colums(0)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MapRows.Empty();
}

bool ASokobanBoard::TryPushBox(AGTWSokobanBox* Box, const FIntPoint& Direction)
{
	if (!IsValid(Box) || Box->IsMoving())
	{
		return false;
	}

	// 상자 현재 격자 위치
	const FIntPoint Current = WorldToGrid(
		Box->GetActorLocation()
	);

	const FIntPoint Next = Current + Direction;

	// 맵 범위 검사
	if (Next.X < 0 || Next.X >= Colums ||
		Next.Y < 0 || Next.Y >= Rows)
	{
		return false;
	}

	// 벽 검사
	if (MapRows[Next.Y][Next.X] == TEXT('W'))
	{
		return false;
	}

	// 다른 상자가 이동할 위치를 차지하는지 검사
	for (AGTWSokobanBox* OtherBox : SpawnedBoxes)
	{
		if (!IsValid(OtherBox) || OtherBox == Box)
		{
			continue;
		}

		if (WorldToGrid(OtherBox->GetActorLocation()) == Next)
		{
			return false;
		}
	}

	// 격자 좌표를 Board 상대 좌표로 변환
	const FVector LocalTarget(
		Next.X * TileSize,
		Next.Y * TileSize,
		0.f
	);

	FVector WorldTarget =
		RootComponent->GetComponentTransform()
		.TransformPosition(LocalTarget);

	// 상자의 현재 높이 유지
	WorldTarget.Z = Box->GetActorLocation().Z;

	return Box->StartMove(WorldTarget);
}

// Called when the game starts or when spawned
void ASokobanBoard::BeginPlay()
{
	Super::BeginPlay();
	if (LoadMapFileData())
	{
		InitializeMap();
	}
}

// Called every frame
void ASokobanBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// Todo : 나중에 게임클리어 시간 기록하려면 Tick필요함
}

bool ASokobanBoard::LoadMapFileData()
{
	FString FilePath = FPaths::Combine(
		FPaths::ProjectContentDir(),
		TEXT("Floor"),
		TEXT("Sokoban"),
		TEXT("MapData"),
		TEXT("SokobanMap.txt")
	);

	if (FFileHelper::LoadFileToString(MapDataText, *FilePath))
	{
		UE_LOG(LogTemp, Warning, TEXT("map load success %s"), *FilePath);

		// 줄바꿈을 기준으로 문자열을 정리
		MapDataText.ParseIntoArrayLines(MapRows, true);

		Rows = MapRows.Num();
		Colums = MapRows[0].Len();

		MapGrid.SetNum(Rows);
		for (int32 i = 0; i < Rows; ++i)
		{
			MapGrid[i].SetNum(Colums);
			for (int32 j = 0; j < Colums; ++j)
			{
				MapGrid[i][j] = MapRows[i][j];
			}
			UE_LOG(LogTemp, Log, TEXT("%s"), *MapRows[i]);
		}

		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("map load fail %s"), *FilePath);
	}

	return false;
}

void ASokobanBoard::InitializeMap()
{
	for (int32 i = 0; i < Rows; ++i)
	{
		for (int32 j = 0; j < Colums; ++j)
		{
			const TCHAR Tile = MapRows[i][j];

			const FVector TileLocation(
				j * TileSize,
				i * TileSize,
				0.f
			);

			UStaticMesh* GoalStaticMesh = nullptr;
			switch (MapRows[i][j])
			{
			case 'W':
				CreateTileMesh(WallStaticMesh, TileLocation);
				break;
			case 'R':
			{
				const UStaticMeshComponent* ResumeGoalComponent =
					AActor::GetActorClassDefaultComponent<UStaticMeshComponent>(ResumeGoalTileClass.Get());

				if (ResumeGoalComponent != nullptr)
				{
					GoalStaticMesh = ResumeGoalComponent->GetStaticMesh();

					UMaterialInterface* GoalMaterial =
						ResumeGoalComponent->GetMaterial(0);

					if (GoalStaticMesh != nullptr)
					{
						CreateTileMesh(GoalStaticMesh, TileLocation, GoalMaterial);
					}
				}
				break;
			}

			case 'G':
			{
				const UStaticMeshComponent* IntroductionGoalComponent =
					AActor::GetActorClassDefaultComponent<UStaticMeshComponent>(IntroductionGoalTileClass.Get());

				if (IntroductionGoalComponent != nullptr)
				{
					GoalStaticMesh = IntroductionGoalComponent->GetStaticMesh();

					UMaterialInterface* GoalMaterial =
						IntroductionGoalComponent->GetMaterial(0);

					if (GoalStaticMesh != nullptr)
					{
						CreateTileMesh(GoalStaticMesh, TileLocation, GoalMaterial);
					}
				}
				break;
			}
			case 'B':
			{
				const UStaticMeshComponent* PortfoliGoalComponent =
					AActor::GetActorClassDefaultComponent<UStaticMeshComponent>(PortfoliGoalTileClass.Get());

				if (PortfoliGoalComponent != nullptr)
				{
					GoalStaticMesh = PortfoliGoalComponent->GetStaticMesh();

					UMaterialInterface* GoalMaterial =
						PortfoliGoalComponent->GetMaterial(0);

					if (GoalStaticMesh != nullptr)
					{
						CreateTileMesh(GoalStaticMesh, TileLocation, GoalMaterial);
					}
				}
				break;
			}
			case 'r':
			{
				SpawnBox(RedBoxClass, TileLocation);
				CreateTileMesh(FloorStaticMesh, TileLocation);
				break;
			}
			case 'g':
			{
				SpawnBox(GreenBoxClass, TileLocation);
				CreateTileMesh(FloorStaticMesh, TileLocation);
				break;
			}
			case 'b':
			{
				SpawnBox(BlueBoxClass, TileLocation);
				CreateTileMesh(FloorStaticMesh, TileLocation);
				break;
			}
			case '.':
				CreateTileMesh(FloorStaticMesh, TileLocation);
				break;
			case 'P':
				CreateTileMesh(FloorStaticMesh, TileLocation);
				break;
			}

		}
	}
}

void ASokobanBoard::CreateTileMesh(UStaticMesh * MeshAsset, const FVector & Location, UMaterialInterface * Material)
{
	if (!MeshAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("Mesh Asset is Null!"));
		return;
	}

	UStaticMeshComponent* MeshComponent =
		NewObject<UStaticMeshComponent>(this);

	MeshComponent->SetStaticMesh(MeshAsset);

	if (Material)
	{
		MeshComponent->SetMaterial(0, Material);
	}

	// Board의 RootComponent에 연결
	MeshComponent->SetupAttachment(RootComponent);

	// Board 기준 상대 위치 지정
	MeshComponent->SetRelativeLocation(Location);

	// 액터가 컴포넌트를 소유하도록 등록
	AddInstanceComponent(MeshComponent);

	// 월드에 컴포넌트 등록
	MeshComponent->RegisterComponent();
}

AGTWSokobanBox* ASokobanBoard::SpawnBox(TSubclassOf<AGTWSokobanBox> BoxClass, const FVector & TileLocation)
{
	if (!BoxClass)
	{
		return nullptr;
	}

	// 블루프린트에 설정된 상자 메시 정보
	const UStaticMeshComponent* BoxMeshComponent =
		AActor::GetActorClassDefaultComponent<UStaticMeshComponent>(
			BoxClass.Get()
		);

	if (!BoxMeshComponent || !BoxMeshComponent->GetStaticMesh())
	{
		return nullptr;
	}

	// 메시의 Z 크기와 블루프린트 스케일 반영
	const FBoxSphereBounds Bounds =
		BoxMeshComponent->GetStaticMesh()->GetBounds();

	const float BoxHalfHeight =
		Bounds.BoxExtent.Z *
		FMath::Abs(BoxMeshComponent->GetRelativeScale3D().Z);

	FVector LocalLocation = TileLocation;
	LocalLocation.Z += BoxHalfHeight;

	// Board 상대 좌표 → 월드 좌표
	const FVector WorldLocation =
		RootComponent->GetComponentTransform()
		.TransformPosition(LocalLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AGTWSokobanBox* Box = GetWorld()->SpawnActor<AGTWSokobanBox>(
		BoxClass.Get(),
		WorldLocation,
		GetActorRotation(),
		SpawnParams
	);

	if (Box)
	{
		SpawnedBoxes.Add(Box);
	}

	return Box;
}

FIntPoint ASokobanBoard::WorldToGrid(const FVector& WorldLocation) const
{
	const FVector LocalLocation =
		RootComponent->GetComponentTransform()
		.InverseTransformPosition(WorldLocation);

	return FIntPoint(
		FMath::RoundToInt(LocalLocation.X / TileSize),
		FMath::RoundToInt(LocalLocation.Y / TileSize)
	);
}
