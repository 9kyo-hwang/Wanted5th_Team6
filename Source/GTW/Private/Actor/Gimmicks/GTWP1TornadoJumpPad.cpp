#include "Actor/Gimmicks/GTWP1TornadoJumpPad.h"

#include "Characters/Boss/GTWBossPhase1Character.h"

void AGTWP1TornadoJumpPad::PostInitializeComponents()
{
	// 원본이 오버랩을 등록하기 전에 INI 수치를 적용한다. 발판마다 파일을 다시 읽지 않는다.
	if (const AGTWBossPhase1Character* Boss = Cast<AGTWBossPhase1Character>(GetOwner()))
	{
		JumpPower = Boss->TornadoEscapePadJumpPower;
	}
	Super::PostInitializeComponents();
}
