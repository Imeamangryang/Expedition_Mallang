// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SlimePlaySaveGame.generated.h"

/**
 * 
 */
UCLASS()
class EXPEDITION_MALLANG_API USlimePlaySaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	// [플레이 저장]
	// 플레이어 스탯 -> Level : 레벨만 저장하면 FPlayerStat으로 불러올 수 있음
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SaveLevel = 1;
	
	// 플레이어 뉴벅스
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SaveNewbucks = 3000;
	
	// 플레이 타임
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SavePlayTime = 0;
	
	// 플레이어 습득 아이템 -> TArray<FSlot> 플레이어가 갖고 있었던 InvenSlot 정보
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<struct FSlot> SaveInvenSlots;
	
	// 슬라임 농장에 있는 슬라임들 -> 슬라임ID * Count => Spawn Slime
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<struct FFarmSlime> SaveFarmSlimeCounts;
};
