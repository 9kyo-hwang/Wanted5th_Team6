#include "Controllers/GTWBoss1PAIController.h"
#include "Characters/Boss/GTWBoss1.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

bool AGTWBoss1PAIController::RunAI()
{
    // 1. 조종 중인 보스 확인. BT 실행 시점은 보스 StartEncounter에서 결정한다.
    auto* Boss = Cast<AGTWBoss1>(GetPawn());
    const bool bHasBoss = Boss != nullptr;
    if (!bHasBoss)
    {
        UE_LOG(LogTemp, Warning, TEXT("P1 AI: 조종 중인 Pawn이 GTWBoss1이 아닙니다."));
        return false;
    }

    // 2. 전투 시작 여부 확인
    const bool bEncounterRunning = Boss->IsEncounterRunning();
    if (!bEncounterRunning)
    {
        UE_LOG(LogTemp, Warning, TEXT("P1 AI: 전투가 시작되지 않았습니다."));
        return false;
    }

    // 3. BP에서 지정한 Blackboard 에셋 확인
    UBlackboardData* Data = BBAsset;
    const bool bHasBlackboardAsset = Data != nullptr;
    if (!bHasBlackboardAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("P1 AI: BP_Boss1PAIController의 BBAsset을 지정해주세요."));
        return false;
    }

    // 4. BP에서 지정한 Behavior Tree 에셋 확인
    UBehaviorTree* Tree = BTAsset;
    const bool bHasBehaviorTreeAsset = Tree != nullptr;
    if (!bHasBehaviorTreeAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("P1 AI: BP_Boss1PAIController의 BTAsset을 지정해주세요."));
        return false;
    }

    // 5. BT가 사용하는 Blackboard와 컨트롤러의 설정이 일치하는지 확인
    const bool bMatchingBlackboard = Tree->BlackboardAsset == Data;
    if (!bMatchingBlackboard)
    {
        UE_LOG(LogTemp, Warning, TEXT("P1 AI: BT의 Blackboard Asset과 BBAsset이 다릅니다."));
        return false;
    }

    // 6. Blackboard 컴포넌트 초기화 결과 확인
    UBlackboardComponent* BlackboardPtr = Blackboard;
    const bool bBlackboardInitialized = UseBlackboard(Data, BlackboardPtr);
    if (!bBlackboardInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("P1 AI: Blackboard 초기화에 실패했습니다."));
        return false;
    }

    // 재시작 시 이전 Task 초기화
    StopAI();
    Blackboard->SetValueAsVector(TEXT("HomePos"), GetPawn()->GetActorLocation());
    Blackboard->SetValueAsVector(TEXT("TargetPos"), GetPawn()->GetActorLocation());

    // Task 실행 전 초기값
    Blackboard->SetValueAsFloat(TEXT("DropInterval"), FMath::Max(0.1f, Boss->DropInterval));

    // 7. Behavior Tree 실행 결과 확인
    const bool bBehaviorTreeStarted = RunBehaviorTree(Tree);
    if (!bBehaviorTreeStarted)
    {
        UE_LOG(LogTemp, Warning, TEXT("P1 AI: Behavior Tree 실행에 실패했습니다."));
    }
    return bBehaviorTreeStarted;
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
