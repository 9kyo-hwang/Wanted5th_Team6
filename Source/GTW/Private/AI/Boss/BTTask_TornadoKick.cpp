#include "AI/Boss/BTTask_TornadoKick.h"

#include "AIController.h"
#include "Characters/Boss/GTWBossPhase1Character.h"

UBTTask_TornadoKick::UBTTask_TornadoKick()
{
	NodeName = TEXT("TornadoKick");
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_TornadoKick::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	AGTWBossPhase1Character* Boss = AI ? Cast<AGTWBossPhase1Character>(AI->GetPawn()) : nullptr;
	if (!Boss || !Boss->IsEncounterRunning())
	{
		return EBTNodeResult::Failed;
	}
	// 기존 BT의 킥 차례 중 일부만 실행한다. 생략도 성공으로 반환해 다음 Wait 흐름을 유지한다.
	const bool bSelected = Boss->TornadoKickChance >= 1.f || FMath::FRand() < Boss->TornadoKickChance;
	if (!bSelected)
	{
		return EBTNodeResult::Succeeded;
	}
	return Boss->PlayTornadoKick() ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
}

void UBTTask_TornadoKick::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	AGTWBossPhase1Character* Boss = AI ? Cast<AGTWBossPhase1Character>(AI->GetPawn()) : nullptr;
	if (!Boss || !Boss->IsEncounterRunning())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
	else if (!Boss->IsPerformingTornadoKick())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}

EBTNodeResult::Type UBTTask_TornadoKick::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AI = OwnerComp.GetAIOwner();
	if (AGTWBossPhase1Character* Boss = AI ? Cast<AGTWBossPhase1Character>(AI->GetPawn()) : nullptr)
	{
		Boss->CancelTornadoKick();
	}
	return EBTNodeResult::Aborted;
}
