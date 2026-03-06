// Fill out your copyright notice in the Description page of Project Settings.


#include "_NZW/SlimeDeadUI.h"

#include "SlimePlayer.h"

void USlimeDeadUI::SetUpSlimePlayer(ASlimePlayer* SlimePlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("DeadUI SetUpSlimePlayer called. Player=%s"), 
	SlimePlayer ? *SlimePlayer->GetName() : TEXT("NULL"));
	
	SlimePlayer->OnDead.AddDynamic(this, &USlimeDeadUI::OnDead);
}

void USlimeDeadUI::OnDead()
{
	UE_LOG(LogTemp, Warning, TEXT("OnDead 델리게이트"));
	BP_Dead_UI();
}
