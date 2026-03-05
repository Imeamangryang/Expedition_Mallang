// Fill out your copyright notice in the Description page of Project Settings.


#include "_NZW/SlimePlayerStatUI.h"

#include "SlimePlayer.h"

void USlimePlayerStatUI::SetUpSlimePlayer(ASlimePlayer* SlimePlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("HUD SetUpSlimePlayer called. Player=%s"),
	SlimePlayer ? *SlimePlayer->GetName() : TEXT("NULL"));
	
	SlimePlayer->OnUpdateHPInPercent.AddDynamic(this, &USlimePlayerStatUI::OnUpdateHPInPercent);
	SlimePlayer->OnUpdateMPInPercent.AddDynamic(this, &USlimePlayerStatUI::OnUpdateMPInPercent);
	SlimePlayer->OnUpdateNewbucks.AddDynamic(this, &USlimePlayerStatUI::OnUpdateNewbucks);
	SlimePlayer->OnShopInteraction.AddDynamic(this, &USlimePlayerStatUI::OnUpdateLevel);
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
