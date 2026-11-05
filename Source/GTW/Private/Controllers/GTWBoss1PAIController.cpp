#include "Controllers/GTWBoss1PAIController.h"
#include "Characters/Boss/GTWBoss1.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

bool AGTWBoss1PAIController::RunAI()
{
    // BT 실행 시점은 보스 StartEncounter에서 결정
    auto* Boss = Cast<AGTWBoss1>(GetPawn());
    if (!Boss || !Boss->IsEncounterRunning())
    {
        return false;
    }
    UBlackboardData* Data = BBAsset;
    UBehaviorTree* Tree = BTAsset;
    UBlackboardComponent* BlackboardPtr = Blackboard;
    if (!Data || !Tree || Tree->BlackboardAsset != Data || !UseBlackboard(Data, BlackboardPtr))
    {
        return false;
    }

    // 재시작 시 이전 Task 초기화
    StopAI();
    Blackboard->SetValueAsVector(TEXT("HomePos"), GetPawn()->GetActorLocation());
    Blackboard->SetValueAsVector(TEXT("TargetPos"), GetPawn()->GetActorLocation());

    // Task 실행 전 초기값
    Blackboard->SetValueAsFloat(TEXT("DropInterval"), FMath::Max(0.1f, Boss->DropInterval));
    return RunBehaviorTree(Tree);
}

void AGTWBoss1PAIController::StopAI()
{
    StopMovement();
    ClearFocus(EAIFocusPriority::Gameplay);
    if (UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(BrainComponent))
    {
        BTComponent->StopTree(EBTStopMode::Forced);
    }
}
