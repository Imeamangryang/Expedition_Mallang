// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VacuumableInfo.h"
#include "SlimeItemDefinition.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class EXPEDITION_MALLANG_API USlimeItemDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EVacuumableType VacType = EVacuumableType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFoodType FoodType = EFoodType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* ImageIcon = nullptr;
};
