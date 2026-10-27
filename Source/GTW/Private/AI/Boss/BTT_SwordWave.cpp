// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Boss/BTT_SwordWave.h"
#include <Characters/GTWBoss.h>
#include <AIController.h>

UBTT_SwordWave::UBTT_SwordWave()
{
	NodeName = TEXT("SwordWave");
}

EBTNodeResult::Type UBTT_SwordWave::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{

	// AI컨트롤러에 연결된 PAwn가져와서 함수 호출
	AGTWBoss* bossPawn = Cast<AGTWBoss>(OwnerComp.GetAIOwner()->GetPawn());
	if (!bossPawn)
		return EBTNodeResult::Failed;

	// 함수 실행
	bossPawn->PlaySwordWave();

	return EBTNodeResult::Succeeded;
}
