// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SlimeGameInstance.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FPlayerStat : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHP;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxMP;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WavePower;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Cost;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOpen;
};

UCLASS()
class EXPEDITION_MALLANG_API USlimeGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	
	// 특정 레벨 데이터 조회
	UFUNCTION(BlueprintCallable, Category="SlimeGame|Stat")
	bool GetStatData(int32 Level, FPlayerStat& OutStat) const;
	
	// 업그레이드 적용 
	UFUNCTION(BlueprintCallable, Category="SlimeGame|Stat")
	bool ApplyUpgrade(int32& CurLevel, FPlayerStat& OutNextStat);
	
/*! 변수 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlimeGame|State")
	int32 CurStatLevel = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlimeGame|State")
	UDataTable* StatData;
};
