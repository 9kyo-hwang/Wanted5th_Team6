// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Composites/BTComposite_Selector.h"
#include "BTComposite_RandomSelector.generated.h"

UCLASS()
class GTW_API UBTComposite_RandomSelector : public UBTComposite_Selector
{
	GENERATED_BODY()
protected:
	virtual int32 GetNextChildHandler(struct FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const override;
	
};
