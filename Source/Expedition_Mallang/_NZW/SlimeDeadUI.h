// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlimeDeadUI.generated.h"

class ASlimePlayer;

/**
 * 
 */
UCLASS()
class EXPEDITION_MALLANG_API USlimeDeadUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetUpSlimePlayer(ASlimePlayer* SlimePlayer);
	
	/** Delegate 연결 함수 */
	UFUNCTION()
	void OnDead();
	
protected:
	/** Blueprint에서 실행 할 함수 */
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "Dead_UI"))
	void BP_Dead_UI();
};
