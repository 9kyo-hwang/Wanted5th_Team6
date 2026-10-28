// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Boss/BTComposite_RandomSelector.h"

int32 UBTComposite_RandomSelector::GetNextChildHandler(
	FBehaviorTreeSearchData& SearchData, int32 PrevChild, 
	EBTNodeResult::Type LastResult) const
{
	// 현재 실행된 자식이 없을때만 랜덤 실행.
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

	// 현재 실행된 자식이 있으면 부모노드로 이동(시퀀스로)
	return BTSpecialChild::ReturnToParent;
}
