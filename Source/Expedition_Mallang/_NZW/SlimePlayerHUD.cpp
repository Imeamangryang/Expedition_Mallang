// Fill out your copyright notice in the Description page of Project Settings.


#include "_NZW/SlimePlayerHUD.h"

#include <string>

#include "SlimePlayer.h"

void USlimePlayerHUD::SetUpSlimePlayer(ASlimePlayer* SlimePlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("HUD SetUpSlimePlayer called. Player=%s"),
	SlimePlayer ? *SlimePlayer->GetName() : TEXT("NULL"));
	
	SlimePlayer->OnUpdateHPInPercent.AddDynamic(this, &USlimePlayerHUD::OnUpdateHPInPercent);
	SlimePlayer->OnUpdateMPInPercent.AddDynamic(this, &USlimePlayerHUD::OnUpdateMPInPercent);
	SlimePlayer->OnUpdateNewbucks.AddDynamic(this, &USlimePlayerHUD::OnUpdateNewbucks);
}

void USlimePlayerHUD::OnUpdateHPInPercent(float Percent)
{
	BP_UpdateHP(Percent);
}

void USlimePlayerHUD::OnUpdateMPInPercent(float Percent)
{
	BP_UpdateMP(Percent);
}

void USlimePlayerHUD::OnUpdateNewbucks(int32 Newbucks)
{
	BP_UpdateNB(Newbucks);
}
