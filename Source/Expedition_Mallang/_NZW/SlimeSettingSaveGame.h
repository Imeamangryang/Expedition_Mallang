// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SlimeSettingSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class EXPEDITION_MALLANG_API USlimeSettingSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	// [설정 저장]
	// 사운드 볼륨 BG, Effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MasterVolume;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BGVolume;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectVolume;
	
	// 감도
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MouseSensitive;
};
