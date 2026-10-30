// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Boss/BTTask_SwordWave.h"
#include <Characters/Boss/GTWBoss.h>
#include <AIController.h>

UBTTask_SwordWave::UBTTask_SwordWave()
{
	NodeName = TEXT("SwordWave");
}

EBTNodeResult::Type UBTTask_SwordWave::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	// AI컨트롤러에 연결된 PAwn가져와서 함수 호출
	AGTWBoss* BossPawn = Cast<AGTWBoss>(OwnerComp.GetAIOwner()->GetPawn());
	if (!BossPawn)
	{
		return EBTNodeResult::Failed;
	}

	// 함수 실행
	BossPawn->PlaySwordWave();

	return EBTNodeResult::Succeeded;
}
