#include "UI/GTWP1BossHitWidget.h"

#include "Characters/Boss/GTWBossPhase1Character.h"

void UGTWP1BossHitWidget::SetBoss(AGTWBossPhase1Character* InBoss)
{
	Boss = InBoss;
	LastHitCount = INDEX_NONE;
	SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UGTWP1BossHitWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!Boss.IsValid())
	{
		return;
	}
	const int32 HitCount = Boss->GetDisplayedHitCount();
	const bool bReacting = Boss->IsReactingToHit();
	const bool bDisplayChanged = HitCount != LastHitCount || bReacting != bLastReacting;
	if (bDisplayChanged)
	{
		LastHitCount = HitCount;
		bLastReacting = bReacting;
		UpdateHitDisplay(HitCount, bReacting);
	}
}
