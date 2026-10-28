// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/GTWBossAIController.h"
#include <BehaviorTree/BlackboardData.h>
#include <BehaviorTree/BlackboardComponent.h>
#include <BehaviorTree/BehaviorTree.h>

AGTWBossAIController::AGTWBossAIController()
{
	// 에셋 가져오기
	static ConstructorHelpers::FObjectFinder<UBlackboardData> BBAssetRef(
		TEXT("/Game/GTW/Boss/AI/BB_P2Boss.BB_P2Boss")
	);
	if (BBAssetRef.Succeeded())
		BBAsset = BBAssetRef.Object;

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTAssetRef(
		TEXT("/Game/GTW/Boss/AI/BT_P2Boss.BT_P2Boss")
	);
	if (BTAssetRef.Succeeded())
		BTAsset = BTAssetRef.Object;


}

void AGTWBossAIController::RunAI()
{
	// 블랙보드 컴포넌트 가져오기(AI컨트롤러 내부에 있음)
	// L-value로 변경하여 가져온다. 
	UBlackboardComponent* BlackboardPtr = Blackboard;
	if (UseBlackboard(BBAsset, BlackboardPtr))
	{
		// 보스 폰이 생성된 위치를 블랙보드 컴포넌트에 기록
		Blackboard->SetValueAsVector(TEXT("HomePos"), GetPawn()->GetActorLocation());
		// BT 실행
		bool RunResult = RunBehaviorTree(BTAsset);
		ensureAlways(RunResult);
	}

}

void AGTWBossAIController::StopAI()
{
	// BT 중지
	UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComponent)
		BTComponent->StopTree();
}

void AGTWBossAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	RunAI();
}
