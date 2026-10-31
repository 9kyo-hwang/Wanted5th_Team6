// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GTWWidgetInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGTWWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GTW_API IGTWWidgetInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// Speed 위젯 함수
	virtual void SetSpeedWidget(class UGTWSpeedWidget* InSpeedWidget) = 0;

};
