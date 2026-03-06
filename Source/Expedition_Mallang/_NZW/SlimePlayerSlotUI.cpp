// Fill out your copyright notice in the Description page of Project Settings.


#include "_NZW/SlimePlayerSlotUI.h"

#include "SlimePlayer.h"

void USlimePlayerSlotUI::SetUpSlimePlayer(ASlimePlayer* SlimePlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("SlotUI SetUpSlimePlayer called. Player=%s"),
	SlimePlayer ? *SlimePlayer->GetName() : TEXT("NULL"));
	
	SlimePlayer->OnVacuuming.AddDynamic(this, &USlimePlayerSlotUI::OnUpdateSlotUI);
	SlimePlayer->OnSelectSlot.AddDynamic(this, &USlimePlayerSlotUI::OnSelectSlot);
	SlimePlayer->OnItemInfo.AddDynamic(this, &USlimePlayerSlotUI::OnItemInfoUI);
}

void USlimePlayerSlotUI::OnSelectSlot(int32 SlotNum, int32 PrevSlotNum)
{
	BP_SelectSlot_UI(SlotNum, PrevSlotNum);
}

void USlimePlayerSlotUI::OnUpdateSlotUI(FName ID, int32 Count, int32 SlotNum)
{
	BP_UpdateSlot_UI(ID, Count, SlotNum);
	
	// switch (SlotNum)
	// {
	// 	case 0: BP_UpdateSlot_1_UI(ID, Count); break;
	// 	case 1: BP_UpdateSlot_2_UI(ID, Count); break;
	// 	case 2: BP_UpdateSlot_3_UI(ID, Count); break;
	// 	case 3: BP_UpdateSlot_4_UI(ID, Count); break;
	// 	default: break;
	// }
}

void USlimePlayerSlotUI::OnItemInfoUI(FName ID)
{
	BP_ItemInfo_UI(ID);
}
