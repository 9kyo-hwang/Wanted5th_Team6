#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "GTWBoss1PAIController.generated.h"

/**
 * 1P 컨트롤러.
 */
UCLASS()
class GTW_API AGTWBoss1PAIController : public AAIController
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category=AI) bool RunAI();
    UFUNCTION(BlueprintCallable, Category=AI) void StopAI();
protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=AI) TObjectPtr<class UBlackboardData> BBAsset;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=AI) TObjectPtr<class UBehaviorTree> BTAsset;
};
