#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_DropObstacles.generated.h"

/** BT_P1Boss의 공격 진입점. 단일 플레이어를 조회하고 보스에게 공격을 요청한다. */
UCLASS()
class GTW_API UBTTask_DropObstacles : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UBTTask_DropObstacles();
	/** 공격 요청만 완료한다. 애니메이션/공격 종료까지 대기하지 않는다. */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
