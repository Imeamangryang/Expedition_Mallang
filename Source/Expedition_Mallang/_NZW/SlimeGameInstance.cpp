// Fill out your copyright notice in the Description page of Project Settings.


#include "_NZW/SlimeGameInstance.h"

void USlimeGameInstance::Init()
{
	Super::Init();
	
	if (StatData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[SlimeGameInstance] State Data Table 로드완료/ 행 수:%d"), StatData->GetRowNames().Num())
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[SlimeGameInstance] StatDataTable 할당안됨"));
	}
}

bool USlimeGameInstance::GetStatData(int32 Level, FPlayerStat& OutStat) const
{
	if (!StatData)
	{
		UE_LOG(LogTemp, Warning, TEXT("StatDataTable 할당안됨"));
		return false;
	}
	
	FString RowName = FString::Printf(TEXT("LV_%d"), Level);
	FPlayerStat* Row = StatData->FindRow<FPlayerStat>(FName(*RowName), TEXT("GetStatData"));
	
	if (!Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s 행을 찾을 수 없음"), *RowName);
		return false;
	}
	
	UE_LOG(LogTemp, Warning, TEXT("%s 행"), *RowName);
	OutStat = *Row;
	return true;
}

bool USlimeGameInstance::ApplyUpgrade(int32& OutCurLevel, FPlayerStat& OutNextStat)
{
	if (CurStatLevel != OutCurLevel)
	{
		UE_LOG(LogTemp, Error, TEXT("!!!! 레벨이 다름 !!!"));
		return false;
	}
	
	int32 NextLevel = CurStatLevel + 1;
	
	if (!GetStatData(NextLevel, OutNextStat))
	{
		UE_LOG(LogTemp, Warning, TEXT("%d 행 데이터 찾을 수 없음"), NextLevel);
		return false;
	}
	
	CurStatLevel = NextLevel;
	OutCurLevel = NextLevel;
	
	UE_LOG(LogTemp, Warning, TEXT("레벨 업 완료: LV %d → LV %d"), CurStatLevel - 1, CurStatLevel);
	return true;
}
