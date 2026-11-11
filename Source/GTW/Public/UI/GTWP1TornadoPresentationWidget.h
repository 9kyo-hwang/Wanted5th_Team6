#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GTWP1TornadoPresentationWidget.generated.h"

class AGTWBossPhase1Character;

/** P1 킥의 표시용 좌표만 전달한다. 이미지·문구·선 그리기는 자식 UMG BP가 담당한다. */
UCLASS()
class GTW_API UGTWP1TornadoPresentationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetBoss(AGTWBossPhase1Character* InBoss);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** BP의 말풍선 페이드 갱신. 표시 시간과 외형은 BP에서 편집한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "P1|Presentation")
	void UpdatePresentation(float ElapsedSeconds);

	/** Designer의 말풍선 영역. 이 영역을 기준으로 보스 방향의 꼬리 좌표를 구한다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "P1|Presentation")
	TObjectPtr<UWidget> SpeechPanel;

	/** AirKick 구간에만 채워지는 화면 좌표. BP OnPaint에서 색·두께를 지정해 그린다. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "P1|Presentation")
	TArray<FVector2D> WindRibbonA;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "P1|Presentation")
	TArray<FVector2D> WindRibbonB;

	UPROPERTY(BlueprintReadOnly, Transient, Category = "P1|Presentation")
	TArray<FVector2D> WindRibbonC;

	/** 고정된 말풍선에서 보스를 향하는 꼬리의 세 점. */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "P1|Presentation")
	TArray<FVector2D> SpeechTail;

private:
	void UpdateScreenPoints(const FGeometry& Geometry);

	TWeakObjectPtr<AGTWBossPhase1Character> Boss;
	float Elapsed = 0.f;
};
