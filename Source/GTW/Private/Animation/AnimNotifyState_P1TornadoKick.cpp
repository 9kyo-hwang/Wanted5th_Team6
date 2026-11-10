#include "Animation/AnimNotifyState_P1TornadoKick.h"

#include "Characters/Boss/GTWBossPhase1Character.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotifyState_P1TornadoKick::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (AGTWBossPhase1Character* Boss = MeshComp ? Cast<AGTWBossPhase1Character>(MeshComp->GetOwner()) : nullptr)
	{
		Boss->BeginTornadoKickDamage();
	}
}

void UAnimNotifyState_P1TornadoKick::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (AGTWBossPhase1Character* Boss = MeshComp ? Cast<AGTWBossPhase1Character>(MeshComp->GetOwner()) : nullptr)
	{
		Boss->UpdateTornadoKickDamage();
	}
}

void UAnimNotifyState_P1TornadoKick::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (AGTWBossPhase1Character* Boss = MeshComp ? Cast<AGTWBossPhase1Character>(MeshComp->GetOwner()) : nullptr)
	{
		Boss->EndTornadoKickDamage();
	}
}
