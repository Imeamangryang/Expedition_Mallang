// Fill out your copyright notice in the Description page of Project Settings.


#include "_NZW/SlimePlayerSlotUI.h"

#include "SlimePlayer.h"
#include "Kismet/GameplayStatics.h"

void USlimePlayerSlotUI::SetUpSlimePlayer(ASlimePlayer* SlimePlayer)
{
	SlimePlayer->OnVacuuming.AddDynamic(this, &USlimePlayerSlotUI::OnUpdateSlotUI);
	SlimePlayer->OnSelectSlot.AddDynamic(this, &USlimePlayerSlotUI::OnSelectSlot);
	SlimePlayer->OnItemInfo.AddDynamic(this, &USlimePlayerSlotUI::OnItemInfoUI);
}

void USlimePlayerSlotUI::OnSelectSlot(int32 SlotNum, int32 PrevSlotNum)
{
	BP_SelectSlot_UI(SlotNum, PrevSlotNum);
	UGameplayStatics::PlaySound2D(this, SlotSound);
}

void USlimePlayerSlotUI::OnUpdateSlotUI(FName ID, int32 Count, int32 SlotNum)
{
	BP_UpdateSlot_UI(ID, Count, SlotNum);
}

void USlimePlayerSlotUI::OnItemInfoUI(FName ID)
{
	BP_ItemInfo_UI(ID);
}
