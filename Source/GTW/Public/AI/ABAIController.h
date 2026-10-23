// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ABAIController.generated.h"

/**
 * 
 */
UCLASS()
class GTW_API AABAIController : public AAIController
{
	GENERATED_BODY()
public:
	AABAIController();

	// BT 시작
	void RunAI();

	// BT 멈추기
	void StopAI();

	// 컨트롤러가 폰에 빙의할 때 호출되는 함수
	virtual void OnPossess(APawn* InPawn) override;

private:
	// BB 에셋
	UPROPERTY(VisibleAnywhere, Category = AI)
	TObjectPtr<class UBlackboardData> BBAsset;
	// bt 에셋
	UPROPERTY(VisibleAnywhere, Category = AI)
	TObjectPtr<class UBehaviorTree> BTAsset;
};
