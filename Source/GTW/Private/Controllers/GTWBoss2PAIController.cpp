// Fill out your copyright notice in the Description page of Project Settings.

#include "Controllers/GTWBoss2PAIController.h"
#include <BehaviorTree/BlackboardData.h>
#include <BehaviorTree/BlackboardComponent.h>
#include <BehaviorTree/BehaviorTree.h>

AGTWBoss2PAIController::AGTWBoss2PAIController()
{
	// 에셋 가져오기
	static ConstructorHelpers::FObjectFinder<UBlackboardData> BBAssetRef(
		TEXT("/Game/GTW/AI/BB_P2Boss.BB_P2Boss")
	);
	if (BBAssetRef.Succeeded())
	{
		BBAsset = BBAssetRef.Object;
	}

	static ConstructorHelpers::FObjectFinder<UBehaviorTree> BTAssetRef(
		TEXT("/Game/GTW/AI/BT_P2Boss.BT_P2Boss")
	);
	if (BTAssetRef.Succeeded())
	{
		BTAsset = BTAssetRef.Object;
	}
}

void AGTWBoss2PAIController::RunAI()
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

void AGTWBoss2PAIController::StopAI()
{
	// BT 중지
	UBehaviorTreeComponent* BTComponent = Cast<UBehaviorTreeComponent>(BrainComponent);
	if (BTComponent)
	{
		BTComponent->StopTree();
	}
}

void AGTWBoss2PAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	//RunAI();	

	/*
	* [교황]: AI 가동 로직을 GM이 컨트롤할 수 있게 StartCombat()으로 통일하려고 합니다
	* 현재 보스 생성자에서 AutoPossess 모드라서 배치 -> Possess -> RunAI()로 자동 호출되는 구조인데,
	* StartCombat에서 Controller 통해서 RunAI()를 직접 호춣기 때문에 2중 호출 구조로 잡힘
	*/
}
