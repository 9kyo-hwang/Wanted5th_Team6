#include "AI/Boss/BTTask_DropObstacles.h"

#include "AIController.h"
#include "Characters/Boss/GTWBoss1.h"
#include "Kismet/GameplayStatics.h"

// BT에서 공격을 요청하는 연결 노드다. 몽타주 재생/기믹 생성 로직은 GTWBoss1에만 둔다.
UBTTask_DropObstacles::UBTTask_DropObstacles()
{
	NodeName = TEXT("DropObstacles");
}

EBTNodeResult::Type UBTTask_DropObstacles::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	if (!AI)
	{
		return EBTNodeResult::Failed;
	}

	AGTWBoss1* Boss = Cast<AGTWBoss1>(AI->GetPawn());
	if (!Boss)
	{
		return EBTNodeResult::Failed;
	}

	// 플레이어 조회
	APawn* Player = UGameplayStatics::GetPlayerPawn(AI, 0);
	if (!IsValid(Player))
	{
		return EBTNodeResult::Failed;
	}

	// 요청 후 바로 성공을 반환하므로 BT는 Wait로 넘어간다.
	// 몽타주 종료까지 기다리는 Task가 아니며, 실제 생성은 보스의 종료 콜백에서 수행한다.
	Boss->PlayDropObstacles(Player);
	return EBTNodeResult::Succeeded;
}
