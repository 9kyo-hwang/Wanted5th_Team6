// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Boss/BTT_Lightning.h"
#include <AIController.h>
#include <Characters/GTWBoss.h>

UBTT_Lightning::UBTT_Lightning()
{
	NodeName = TEXT("Lightning");
	UE_LOG(LogTemp, Log, TEXT("Start Lightning"));
}

EBTNodeResult::Type UBTT_Lightning::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 태스크 실행 후 결과 반환
	UE_LOG(LogTemp, Log, TEXT("Start Lightning"));
	// AI컨트롤러에 연결된 PAwn가져와서 함수 호출
	AGTWBoss* bossPawn = Cast<AGTWBoss>(OwnerComp.GetAIOwner()->GetPawn());
	if (!bossPawn)
		return EBTNodeResult::Failed;

	// 함수 실행
	bossPawn->PlayLightning();
	return EBTNodeResult::Succeeded;
}
