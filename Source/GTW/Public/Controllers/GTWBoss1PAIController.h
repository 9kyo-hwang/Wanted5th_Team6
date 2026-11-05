#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "GTWBoss1PAIController.generated.h"

/**
 * P1 보스가 사용하는 AIController.
 * BB/BT 에셋은 BP_Boss1PAIController에서 선택하고, C++은 초기화와 실행/중지를 담당
 * 추격 요청은 BP Service, 공격 요청은 BT Task, 실제 이동/공격은 GTWBoss1에서 처리
 */
UCLASS()
class GTW_API AGTWBoss1PAIController : public AAIController
{
    GENERATED_BODY()
public:
    /**
     * 보스의 전투 시작 요청
     */
    UFUNCTION(BlueprintCallable, Category=AI) 
    bool RunAI();

    /** 
     * 실행 중인 BT 중지
     */
    UFUNCTION(BlueprintCallable, Category=AI) 
    void StopAI();
protected:
    /* 목표 위치와 공격 간격 등 AI가 사용할 데이터를 저장 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=AI) 
    TObjectPtr<class UBlackboardData> BBAsset;

    /* 추격 Service와 공격 Task, 대기 순서를 구성한 BT 에셋 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=AI) 
    TObjectPtr<class UBehaviorTree> BTAsset;
};
