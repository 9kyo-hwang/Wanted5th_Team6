#include "UI/GTWP1TornadoPresentationWidget.h"

#include "Animation/AnimInstance.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Camera/PlayerCameraManager.h"
#include "Characters/Boss/GTWBossPhase1Character.h"
#include "Components/SkeletalMeshComponent.h"

void UGTWP1TornadoPresentationWidget::SetBoss(AGTWBossPhase1Character* InBoss)
{
	Boss = InBoss;
	Elapsed = 0.f;
}

void UGTWP1TornadoPresentationWidget::NativeConstruct()
{
	Super::NativeConstruct();
	// 표시 전용: 플레이어 입력을 가로채지 않는다.
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UGTWP1TornadoPresentationWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!Boss.IsValid() || !Boss->IsPerformingTornadoKick() || !Boss->IsEncounterRunning())
	{
		RemoveFromParent();
		return;
	}
	Elapsed += InDeltaTime;
	UpdateScreenPoints(MyGeometry);
	UpdatePresentation(Elapsed);
}

void UGTWP1TornadoPresentationWidget::UpdateScreenPoints(const FGeometry& Geometry)
{
	WindRibbonA.Reset(49);
	WindRibbonB.Reset(49);
	WindRibbonC.Reset(49);
	SpeechTail.Reset(3);
	if (!GetOwningPlayer())
	{
		return;
	}
	const USkeletalMeshComponent* Mesh = Boss->GetMesh();
	auto Project = [this](const FVector& Position, FVector2D& Screen)
	{
		return UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(GetOwningPlayer(), Position, Screen, true);
	};

	// 화면에 선을 그리지 않고 나선의 월드 좌표를 투영한다. 외형은 BP OnPaint에서 결정한다.
	const UAnimInstance* Anim = Mesh->GetAnimInstance();
	if (Anim && Anim->Montage_GetCurrentSection() == TEXT("AirKick"))
	{
		const FVector Center = Mesh->Bounds.Origin;
		const float Radius = FMath::Max(100.f, Mesh->Bounds.BoxExtent.Size2D() * 0.65f);
		TArray<FVector2D>* Ribbons[] = { &WindRibbonA, &WindRibbonB, &WindRibbonC };
		for (int32 Ribbon = 0; Ribbon < 3; ++Ribbon)
		{
			for (int32 Index = 0; Index <= 48; ++Index)
			{
				const float Alpha = Index / 48.f;
				const float Angle = Alpha * 2.f * PI + Elapsed * 5.f + Ribbon * 2.f * PI / 3.f;
				const FVector Position = Center + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius,
					(Alpha - 0.5f) * Mesh->Bounds.BoxExtent.Z);
				FVector2D Screen;
				if (Project(Position, Screen))
				{
					Ribbons[Ribbon]->Add(Screen);
				}
			}
		}
	}

	if (!SpeechPanel)
	{
		return;
	}
	// Designer 배치를 그대로 사용한다. C++에서 말풍선 위치·크기·색상은 덮어쓰지 않는다.
	const FGeometry& BubbleGeometry = SpeechPanel->GetCachedGeometry();
	const FVector2D BoxSize = BubbleGeometry.GetLocalSize();
	const FVector2D Center = Geometry.AbsoluteToLocal(BubbleGeometry.LocalToAbsolute(BoxSize * 0.5f));
	FVector2D BossScreen;
	FVector2D Direction(0.f, -1.f);
	if (Project(Mesh->GetSocketLocation(TEXT("Head")), BossScreen))
	{
		Direction = (BossScreen - Center).GetSafeNormal();
	}
	else if (const APlayerCameraManager* Camera = GetOwningPlayer()->PlayerCameraManager)
	{
		const FVector ToBoss = Boss->GetActorLocation() - Camera->GetCameraLocation();
		const FRotationMatrix Axes(Camera->GetCameraRotation());
		Direction = FVector2D(FVector::DotProduct(ToBoss, Axes.GetUnitAxis(EAxis::Y)),
			-FVector::DotProduct(ToBoss, Axes.GetUnitAxis(EAxis::Z))).GetSafeNormal();
	}
	if (Direction.IsNearlyZero())
	{
		Direction = FVector2D(0.f, -1.f);
	}
	const FVector2D Edge = Center + FVector2D(Direction.X * BoxSize.X * 0.38f, Direction.Y * BoxSize.Y * 0.32f);
	const FVector2D Side(-Direction.Y * 14.f, Direction.X * 14.f);
	SpeechTail = { Edge + Side, Edge + Direction * 54.f, Edge - Side };
}
