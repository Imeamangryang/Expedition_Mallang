// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlimePlayerHUD.generated.h"

class ASlimePlayer;

UCLASS()
class EXPEDITION_MALLANG_API USlimePlayerHUD : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetUpSlimePlayer(ASlimePlayer* SlimePlayer);
	
	/** Delegate 연결 함수 */
	UFUNCTION()
	void OnUpdateHPInPercent(float Percent);
	UFUNCTION()
	void OnUpdateMPInPercent(float Percent);
	UFUNCTION()
	void OnUpdateNewbucks(int Newbucks);
	
protected:
	/** Blueprint에서 실행 할 함수 */
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "Update HP"))
	void BP_UpdateHP(float Percent);
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "Update MP"))
	void BP_UpdateMP(float Percent);
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "Update NB"))
	void BP_UpdateNB(int Newbucks);
};
