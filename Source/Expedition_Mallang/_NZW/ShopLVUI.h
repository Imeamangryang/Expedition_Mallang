// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopLVUI.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS(Blueprintable)
class EXPEDITION_MALLANG_API UShopLVUI : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	UTextBlock* Text_Level;
};
