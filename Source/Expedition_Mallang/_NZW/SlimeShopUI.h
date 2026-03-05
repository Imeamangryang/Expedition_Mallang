// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlimeShopUI.generated.h"

struct FPlayerStat;

class ASlimePlayer;
class UShopInfoUI;
class UShopLVUI;
class UTextBlock;
/**
 * 
 */
UCLASS()
class EXPEDITION_MALLANG_API USlimeShopUI : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetUpSlimePlayer(ASlimePlayer* SlimePlayer);
	
	UFUNCTION()
	void OnShopInteraction(int32 Level);
	
	UFUNCTION()
	void OnNotEnoughNewbucks();
		
protected:
	/** Blueprint에서 실행 할 함수 */
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "ShopInteraction"))
	void BP_ShopInteraction(FPlayerStat OutCurStat, FPlayerStat OutNextStat);
	
	UFUNCTION(BlueprintImplementableEvent, Category="Slime", meta = (DisplayName = "NotEnoughNewbucks"))
	void BP_NotEnoughNewbucks();
	
public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	UShopInfoUI* ShopInfoUI_Cur;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	UShopInfoUI* ShopInfoUI_Next;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	UShopLVUI* Level_Cur;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	UShopLVUI* Level_Next;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	UTextBlock* Text_Cost;
};
