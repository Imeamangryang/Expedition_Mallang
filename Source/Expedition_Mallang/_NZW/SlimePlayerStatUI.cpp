// Fill out your copyright notice in the Description page of Project Settings.


#include "_NZW/SlimePlayerStatUI.h"

#include "SlimePlayer.h"

void USlimePlayerStatUI::SetUpSlimePlayer(ASlimePlayer* SlimePlayer)
{
	SlimePlayer->OnUpdateHPInPercent.AddDynamic(this, &USlimePlayerStatUI::OnUpdateHPInPercent);
	SlimePlayer->OnUpdateMPInPercent.AddDynamic(this, &USlimePlayerStatUI::OnUpdateMPInPercent);
	SlimePlayer->OnUpdateNewbucks.AddDynamic(this, &USlimePlayerStatUI::OnUpdateNewbucks);
	SlimePlayer->OnShopInteraction.AddDynamic(this, &USlimePlayerStatUI::OnUpdateLevel);
	SlimePlayer->OnUpdatePlayTime.AddDynamic(this, &USlimePlayerStatUI::OnUpdatePlayTime);
}

void USlimePlayerStatUI::OnUpdateHPInPercent(float Percent, float Max)
{
	BP_UpdateHP(Percent, Max);
}

void USlimePlayerStatUI::OnUpdateMPInPercent(float Percent, float Max)
{
	BP_UpdateMP(Percent, Max);
}

void USlimePlayerStatUI::OnUpdateNewbucks(int32 Newbucks)
{
	BP_UpdateNB(Newbucks);
}

void USlimePlayerStatUI::OnUpdateLevel(int32 Level)
{
	BP_UpdateLV(Level);
}

void USlimePlayerStatUI::OnUpdatePlayTime(float PlayTime)
{
	int32 TotalSec = (int32)PlayTime;
	int32 H = TotalSec / 3600;
	int32 M = (TotalSec % 3600) / 60;
	int32 S = TotalSec % 60;
	
	FString CurTime = FString::Printf(TEXT("%02d:%02d:%02d"), H, M, S);
	BP_UpdatePT(FText::FromString(CurTime));
}
