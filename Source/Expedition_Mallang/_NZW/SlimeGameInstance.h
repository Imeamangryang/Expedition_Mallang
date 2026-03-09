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
	int32 Level = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHP = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxMP = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WavePower = 2000;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Cost = 2000;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOpen = true;
};

USTRUCT(BlueprintType)
struct FFarmSlime
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName SlimeID;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AActor> SlimeClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FVector> SlimeLastLocations;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Count;
};

UCLASS()
class EXPEDITION_MALLANG_API USlimeGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;
	virtual void Shutdown() override;
	
	UFUNCTION(BlueprintCallable, Category="SlimeGame|Save&Load")
	void SaveGame();
	UFUNCTION(BlueprintCallable, Category="SlimeGame|Save&Load")
	void LoadGame();
	UFUNCTION(BlueprintCallable, Category="SlimeGame|Save&Load")
	void SaveSettings();
	UFUNCTION(BlueprintCallable, Category="SlimeGame|Save&Load")
	void LoadSettings();
	
	UFUNCTION(BlueprintCallable, Category="SlimeGame|SlimeFarm")
	void SetFarmSlimes(FName SlimeID, TSubclassOf<AActor> SlimeClass, bool bIsIn);
	UFUNCTION(BlueprintCallable, Category="SlimeGame|SlimeFarm")
	void SetFarmSlimeLocation(FName SlimeID, FVector LastLocation);
	UFUNCTION(BlueprintCallable, Category="SlimeGame|SlimeFarm")
	void ClearFarmSlimeLocation();
	
	// 특정 레벨 데이터 조회
	UFUNCTION(BlueprintCallable, Category="SlimeGame|Stat")
	bool GetStatData(int32 Level, FPlayerStat& OutStat) const;
	
	// 업그레이드 적용 
	UFUNCTION(BlueprintCallable, Category="SlimeGame|Stat")
	bool ApplyUpgrade(int32& CurLevel, FPlayerStat& OutNextStat);
	
/*! 변수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USlimePlaySaveGame* PlaySaveGame;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class USlimeSettingSaveGame* SettingSaveGame;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlimeGame|State")
	int32 CurStatLevel = 1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlimeGame|State")
	float SessionStartTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlimeGame|State")
	UDataTable* StatData;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SlimeGame|State")
	UDataTable* VacuumableData;
	
};
