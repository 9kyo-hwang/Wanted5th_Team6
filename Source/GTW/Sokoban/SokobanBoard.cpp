// Fill out your copyright notice in the Description page of Project Settings.


#include "SokobanBoard.h"
#include "GTWSokobanBox.h"

#include <Misc/FileHelper.h>
#include <Misc/Paths.h>
#include <Engine/StaticMesh.h>
#include "Camera/CameraComponent.h"
#include <Components/StaticMeshComponent.h>

#include "TimerManager.h"

// Sets default values
ASokobanBoard::ASokobanBoard()
	: GoalCount(0), Rows(0), Colums(0)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	MapRows.Empty();

	RootComponent = CreateDefaultSubobject<USceneComponent>(
		TEXT("SceneRoot"));

	SokobanBoardCamera = CreateDefaultSubobject<UCameraComponent>(
		TEXT("SokobanCamera"));

	SokobanBoardCamera->SetupAttachment(RootComponent);

	// 직교 카메라 설정
	SokobanBoardCamera->ProjectionMode =
		ECameraProjectionMode::Perspective;
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
		//SetActorHiddenInGame(true);
		GetMapCenterLocation();
	}
}

// Called every frame
void ASokobanBoard::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (bIsSokobanClear == false && bIsSokobanField)
	{
		ElapsedTime += DeltaTime;
	}
}

FVector ASokobanBoard::GetMapCenterLocation() const
{
	const float CenterX = (Colums - 1) * TileSize * 0.5f;
	const float CenterY = (Rows - 1) * TileSize * 0.5f;

	const FVector LocalCenter(CenterX, CenterY, 0.f);

	return RootComponent->GetComponentTransform()
		.TransformPosition(LocalCenter);
}

void ASokobanBoard::AddGoalCount()
{
	GoalCount++;
	if (GoalCount >= MaxGoalCount)
	{
		GameClear();
	}


	GetWorldTimerManager().SetTimerForNextTick(
		this, &ASokobanBoard::ResetGame
	);
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
			const 
			UStaticMesh* GoalStaticMesh = nullptr;
			switch (MapRows[i][j])
			{
			case 'W':
				CreateTileMesh(WallStaticMesh, TileLocation);
				break;
			case 'R':
			{
				SpawnGoalTile(ResumeGoalTileClass, TileLocation);
				break;
			}

			case 'G':
			{
				SpawnGoalTile(IntroductionGoalTileClass, TileLocation);
				break;
			}
			case 'B':
			{
				SpawnGoalTile(PortfoliGoalTileClass, TileLocation);
				break;
			}
			case 'r':
			{
				SpawnBox(RedBoxClass, TileLocation);
				BoxStartLocatin.Add(TileLocation);
				CreateTileMesh(FloorStaticMesh, TileLocation);
				break;
			}
			case 'g':
			{
				SpawnBox(GreenBoxClass, TileLocation);
				BoxStartLocatin.Add(TileLocation);
				CreateTileMesh(FloorStaticMesh, TileLocation);
				break;
			}
			case 'b':
			{
				SpawnBox(BlueBoxClass, TileLocation);
				BoxStartLocatin.Add(TileLocation);
				CreateTileMesh(FloorStaticMesh, TileLocation);
				break;
			}
			case '.':
				CreateTileMesh(FloorStaticMesh, TileLocation);
				break;
			case 'P':
				// Board 기준 위치 → 월드 위치
				PlayerStartLocation =
					RootComponent->GetComponentTransform()
					.TransformPosition(TileLocation);
				CreateTileMesh(FloorStaticMesh, TileLocation);
				break;
			}
		}
	}

	const float CenterX = (Colums - 1) * TileSize * 0.5f;
	const float CenterY = (Rows - 1) * TileSize * 0.5f;

	SokobanBoardCamera->SetRelativeLocation(
		FVector(CenterX, CenterY, 1800.f)
	);

	SokobanBoardCamera->SetRelativeRotation(
		FRotator(-90.f, -90.f, 0.f)
	);
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

AActor* ASokobanBoard::SpawnGoalTile(TSubclassOf<AActor> BoxClass, const FVector& TileLocation)
{
	const FVector WorldLocation =
		RootComponent->GetComponentTransform()
		.TransformPosition(TileLocation);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	AActor* GoalTile = GetWorld()->SpawnActor<AActor>(
		BoxClass.Get(),
		WorldLocation,
		GetActorRotation(),
		SpawnParams);

	return GoalTile;
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

void ASokobanBoard::ResetGame()
{
	for (AGTWSokobanBox* Box : SpawnedBoxes)
	{
		if (IsValid(Box))
		{
			Box->Destroy();
		}
	}

	SpawnedBoxes.Empty();
	GoalCount = 0;
	ElapsedTime = 0.0f;
	bIsSokobanClear = false;

	SpawnBox(RedBoxClass, BoxStartLocatin[0]);
	SpawnBox(GreenBoxClass, BoxStartLocatin[1]);
	SpawnBox(BlueBoxClass, BoxStartLocatin[2]);
}

void ASokobanBoard::GameClear()
{
	bIsSokobanClear = true;
	bIsSokobanField = false;
	int32 SokobanClearTime = ElapsedTime;
	UE_LOG(LogTemp, Error, TEXT("SokobanGame Clear! elaspTime : %d"), SokobanClearTime);
}
