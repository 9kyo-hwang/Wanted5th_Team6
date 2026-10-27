// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Boss/BTC_RandomSelector.h"

#define DEBUG_MODE 1

int32 UBTC_RandomSelector::GetNextChildHandler(
	FBehaviorTreeSearchData& SearchData, int32 PrevChild, 
	EBTNodeResult::Type LastResult) const
{
	// 현재 실행된 자식이 없을때만 랜덤 실행.
#if DEBUG_MODE
	if (PrevChild == BTSpecialChild::NotInitialized)
	{
		int Range = FMath::RandRange(0, 99);
		// 검기: 0 ~ 40(40%)
		if (Range < 40)
			return 0;
		// 번개: 40 ~ 70(30%)
		else if (Range < 70)
			return 1;
		// 공: 70 ~ 100(30%)
		else
			return 2;
	}
#else
	if (PrevChild == BTSpecialChild::NotInitialized)
		return 1;
#endif

	// 현재 실행된 자식이 있으면 부모노드로 ㄱㄱ(시퀀스로간다는뜻)
	return BTSpecialChild::ReturnToParent;
}
