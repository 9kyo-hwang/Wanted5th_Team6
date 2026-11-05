#include "AI/Boss/BTTask_DropObstacles.h"
#include "Controllers/GTWBoss1PAIController.h"
#include "Characters/Boss/GTWBoss1.h"
#include "Kismet/GameplayStatics.h"

/*
* 보스에게 공격 요청 Task
*/
UBTTask_DropObstacles::UBTTask_DropObstacles() 
{ 
    NodeName = TEXT("DropObstacles"); 
}

EBTNodeResult::Type UBTTask_DropObstacles::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AGTWBoss1PAIController* AI = Cast<AGTWBoss1PAIController>(OwnerComp.GetAIOwner());
    if (!AI)
    {
        return EBTNodeResult::Failed;
    }
    
    AGTWBoss1* Boss = AI ? Cast<AGTWBoss1>(AI->GetPawn()) : nullptr;
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

    // 보스에게 공격 요청
    Boss->PlayDropObstacles(Player);
    return EBTNodeResult::Succeeded;
}
