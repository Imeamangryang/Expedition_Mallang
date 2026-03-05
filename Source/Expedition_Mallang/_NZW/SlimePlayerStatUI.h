// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlimePlayerStatUI.generated.h"

class ASlimePlayer;

UCLASS()
class EXPEDITION_MALLANG_API USlimePlayerStatUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetUpSlimePlayer(ASlimePlayer* SlimePlayer);
	
	/** Delegate 연결 함수 */
	UFUNCTION()
	void OnUpdateHPInPercent(float Percent, float Max);
	UFUNCTION()
	void OnUpdateMPInPercent(float Percent, float Max);
	UFUNCTION()
	void OnUpdateNewbucks(int32 Newbucks);
	UFUNCTION()
	void OnUpdateLevel(int32 Level);
	
protected:
	/** Blueprint에서 실행 할 함수 */
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "Update HP"))
	void BP_UpdateHP(float Percent, float Max);
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "Update MP"))
	void BP_UpdateMP(float Percent, float Max);
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "Update NB"))
	void BP_UpdateNB(int32 Newbucks);
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "Update LV"))
	void BP_UpdateLV(int32 Level);
};
