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
	return Boss && Boss->PlayTornadoKick() ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
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
