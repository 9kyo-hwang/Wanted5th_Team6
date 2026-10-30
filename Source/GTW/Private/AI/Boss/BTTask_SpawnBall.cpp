// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/Boss/BTTask_SpawnBall.h"
#include <Characters/Boss/GTWBoss.h>
#include <BehaviorTree/BehaviorTreeComponent.h>
#include <AIController.h>

UBTTask_SpawnBall::UBTTask_SpawnBall()
{
	NodeName = TEXT("SpawnBall");
}

EBTNodeResult::Type UBTTask_SpawnBall::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	EBTNodeResult::Type Result = Super::ExecuteTask(OwnerComp, NodeMemory);

	// AI컨트롤러에 연결된 PAwn가져와서 함수 호출
	AGTWBoss* BossPawn = Cast<AGTWBoss>(OwnerComp.GetAIOwner()->GetPawn());
	if (!BossPawn)
	{
		return EBTNodeResult::Failed;
	}

	// 함수 실행
	BossPawn->PlaySpawnBall();

	// 태스크 실행 후 결과 반환
	//UE_LOG(LogTemp, Log, TEXT("Start SpawnBall"));
	return EBTNodeResult::Succeeded;
}
