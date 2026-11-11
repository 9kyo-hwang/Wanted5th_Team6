#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GTWP1BossHitWidget.generated.h"
class AGTWBossPhase1Character;

/** 적중 알림을 받은 P1 보스의 표시 상태만 읽는다. 게임 클리어를 판단하지 않는다. */
UCLASS()
class GTW_API UGTWP1BossHitWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetBoss(AGTWBossPhase1Character* InBoss);
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** BP는 이 이벤트 하나에서 숫자·남은 칸·반응 문구만 갱신한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "P1|Display")
	void UpdateHitDisplay(int32 HitCount, int32 RequiredCount, bool bReacting);
private:
	TWeakObjectPtr<AGTWBossPhase1Character> Boss;
	int32 LastHitCount = INDEX_NONE;
	bool bLastReacting = false;
};
