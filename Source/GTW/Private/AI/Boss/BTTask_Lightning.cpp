// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Boss/BTTask_Lightning.h"
#include <AIController.h>
#include <Characters/Boss/GTWBoss.h>

UBTTask_Lightning::UBTTask_Lightning()
{
	NodeName = TEXT("Lightning");
	UE_LOG(LogTemp, Log, TEXT("Start Lightning"));
}

EBTNodeResult::Type UBTTask_Lightning::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 태스크 실행 후 결과 반환
	UE_LOG(LogTemp, Log, TEXT("Start Lightning"));
	// AI컨트롤러에 연결된 PAwn가져와서 함수 호출
	AGTWBoss* BossPawn = Cast<AGTWBoss>(OwnerComp.GetAIOwner()->GetPawn());
	if (!BossPawn)
	{
		return EBTNodeResult::Failed;
	}

	// 함수 실행
	BossPawn->PlayLightning();
	return EBTNodeResult::Succeeded;
}
