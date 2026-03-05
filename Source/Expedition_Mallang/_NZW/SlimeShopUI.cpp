// Fill out your copyright notice in the Description page of Project Settings.


#include "_NZW/SlimeShopUI.h"

#include "ShopInfoUI.h"
#include "ShopLVUI.h"
#include "SlimePlayer.h"
#include "SlimeGameInstance.h"
#include "Components/TextBlock.h"

void USlimeShopUI::SetUpSlimePlayer(ASlimePlayer* SlimePlayer)
{
	SlimePlayer->OnShopInteraction.AddDynamic(this, &USlimeShopUI::OnShopInteraction);
	SlimePlayer->OnShopNotEnoughNewbucks.AddDynamic(this, &USlimeShopUI::OnNotEnoughNewbucks);
}

void USlimeShopUI::OnShopInteraction(int32 Level)
{
	USlimeGameInstance* GameInstance = Cast<USlimeGameInstance>(GetGameInstance());
	
	FPlayerStat OutCurStat;
	FPlayerStat OutNextStat;
	
	GameInstance->GetStatData(Level, OutCurStat);
	if (!GameInstance->GetStatData(Level+1, OutNextStat))
	{
		OutNextStat.Level = 0;
		OutNextStat.MaxHP = 0;
		OutNextStat.MaxMP = 0;
		OutNextStat.WavePower = 0;
		OutNextStat.Cost = 0;
	}
	
	Level_Cur->Text_Level->SetText(FText::AsNumber(OutCurStat.Level));
	ShopInfoUI_Cur->Text_1_1->SetText(FText::AsNumber(OutCurStat.MaxHP));
	ShopInfoUI_Cur->Text_2_2->SetText(FText::AsNumber(OutCurStat.MaxMP));
	ShopInfoUI_Cur->Text_3_3->SetText(FText::AsNumber(OutCurStat.WavePower));
	
	Level_Next->Text_Level->SetText(FText::AsNumber(OutNextStat.Level));
	ShopInfoUI_Next->Text_1_1->SetText(FText::AsNumber(OutNextStat.MaxHP));
	ShopInfoUI_Next->Text_2_2->SetText(FText::AsNumber(OutNextStat.MaxMP));
	ShopInfoUI_Next->Text_3_3->SetText(FText::AsNumber(OutNextStat.WavePower));
	
	Text_Cost->SetText(FText::AsNumber(OutNextStat.Cost));
	
	BP_ShopInteraction(OutCurStat,OutNextStat);
}

void USlimeShopUI::OnNotEnoughNewbucks()
{
	BP_NotEnoughNewbucks();
}
