// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlimePlayerSlotUI.generated.h"

class ASlimePlayer;
 
UCLASS()
class EXPEDITION_MALLANG_API USlimePlayerSlotUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetUpSlimePlayer(ASlimePlayer* SlimePlayer);
	
	/** Delegate 연결 함수 */
	UFUNCTION()
	void OnSelectSlot(int32 SlotNum, int32 PrevSlotNum);
	UFUNCTION()
	void OnUpdateSlotUI(FName ID, int32 Count, int32 SlotNum);
	UFUNCTION()
	void OnItemInfoUI(FName ID);
		
protected:
	/** Blueprint에서 실행 할 함수 */
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "SelectSlot_UI"))
	void BP_SelectSlot_UI(int32 SlotNum, int32 PrevSlotNum);
	
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "Update Slot UI"))
	void BP_UpdateSlot_UI(FName ID, int32 Count, int32 SlotNum);
	
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "ItemInfo UI"))
	void BP_ItemInfo_UI(FName ID);
	
	// 과거 함수 
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "Update Slot_1 UI"))
	void BP_UpdateSlot_1_UI(FName ID, int32 Count);
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "Update Slot_2 UI"))
	void BP_UpdateSlot_2_UI(FName ID, int32 Count);
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "Update Slot_3 UI"))
	void BP_UpdateSlot_3_UI(FName ID, int32 Count);
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "Update Slot_4 UI"))
	void BP_UpdateSlot_4_UI(FName ID, int32 Count);
};
