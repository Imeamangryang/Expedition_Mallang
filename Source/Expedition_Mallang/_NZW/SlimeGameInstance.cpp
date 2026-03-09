// Fill out your copyright notice in the Description page of Project Settings.


#include "_NZW/SlimeGameInstance.h"

#include "SlimePlaySaveGame.h"
#include "SlimeSettingSaveGame.h"
#include "VacuumableInfo.h"
#include "Kismet/GameplayStatics.h"

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
	
	LoadGame();
	LoadSettings();
}

void USlimeGameInstance::Shutdown()
{
	Super::Shutdown();
	
	SaveGame();
	SaveSettings();
}

void USlimeGameInstance::SaveGame()
{
	if (PlaySaveGame)
	{
		if (UGameplayStatics::SaveGameToSlot(PlaySaveGame, TEXT("PlaySaveGame"), 0))
		{
			UE_LOG(LogTemp, Warning, TEXT("SaveLevel: %d / SaveNewbucks: %d / SavePlayTime: %.1f / SlimeFarm: %d"), 
				PlaySaveGame->SaveLevel, PlaySaveGame->SaveNewbucks, PlaySaveGame->SavePlayTime, PlaySaveGame->SaveFarmSlimes.Num());
			for (auto Slime : PlaySaveGame->SaveFarmSlimes)
			{
				UE_LOG(LogTemp, Warning, TEXT("SlimeID: %s / SlimeClass: %s / SlimeCount: %d"),
					*Slime.SlimeID.ToString(), Slime.SlimeClass ? *Slime.SlimeClass->GetName() : TEXT("None"), Slime.Count);	
			}
			UE_LOG(LogTemp, Warning, TEXT("SAVE SAVE SAVE SAVE SAVE"));
		}
	}
}

void USlimeGameInstance::LoadGame()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("PlaySaveGame"), 0))
	{	// 저장된 데이터가 있다면
		PlaySaveGame = Cast<USlimePlaySaveGame>(
			UGameplayStatics::LoadGameFromSlot(TEXT("PlaySaveGame"), 0));
	}
	else
	{	// 처음 실행한 상태라면
		PlaySaveGame = Cast<USlimePlaySaveGame>(
			UGameplayStatics::CreateSaveGameObject(USlimePlaySaveGame::StaticClass()));
		
		// 슬롯 초기화
		PlaySaveGame->SaveInvenSlots.SetNum(4);
		
		UGameplayStatics::SaveGameToSlot(PlaySaveGame, TEXT("PlaySaveGame"), 0);
	}
	
	SessionStartTime = GetWorld()->GetTimeSeconds();
	
	if (PlaySaveGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlaySave 생성 됨"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlaySave 생성 안됨!!!"));
	}
}

void USlimeGameInstance::SaveSettings()
{
	if (SettingSaveGame)
	{
		UGameplayStatics::SaveGameToSlot(SettingSaveGame, TEXT("Settings"), 0);
	}
}

void USlimeGameInstance::LoadSettings()
{
	if (UGameplayStatics::DoesSaveGameExist(TEXT("Settings"), 0))
	{	// 저장된 데이터가 있다면
		SettingSaveGame = Cast<USlimeSettingSaveGame>(
			UGameplayStatics::LoadGameFromSlot(TEXT("Settings"), 0));
	}
	else
	{	// 처음 실행한 상태라면 
		SettingSaveGame = Cast<USlimeSettingSaveGame>(
			UGameplayStatics::CreateSaveGameObject(USlimeSettingSaveGame::StaticClass()));
		
		UGameplayStatics::SaveGameToSlot(SettingSaveGame, TEXT("Settings"), 0);
	}
	
	if (SettingSaveGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("SettingSaveGame 생성 됨"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SettingSaveGame 생성 안됨!!!"));
	}
}

void USlimeGameInstance::SetFarmSlimes(FName SlimeID, TSubclassOf<AActor> SlimeClass, bool bIsIn)
{
	//. 해당 슬라임 아이디가 있는 경우
	for (int32 i = 0; i < PlaySaveGame->SaveFarmSlimes.Num(); i++)
	{
		auto& Slime = PlaySaveGame->SaveFarmSlimes[i];
		
		if (Slime.SlimeID == SlimeID)
		{
			if (bIsIn)
			{
				Slime.Count++;
			}
			else
			{
				if (Slime.Count > 0)
				{
					Slime.Count--;
				}
				
				if (Slime.Count <= 0)
				{
					PlaySaveGame->SaveFarmSlimes.RemoveAt(i);
				}
			}
			
			UE_LOG(LogTemp, Warning, TEXT("%d"), PlaySaveGame->SaveFarmSlimes.Num());
		
			return;
		}
	}
	
	// 새로운 아이디의 슬라임이 들어온 경우
	if (bIsIn)
	{
		FFarmSlime FarmSlime;
		FarmSlime.SlimeID = SlimeID;
		FarmSlime.SlimeClass = SlimeClass;
		FarmSlime.Count = 1;
		PlaySaveGame->SaveFarmSlimes.Add(FarmSlime);
		UE_LOG(LogTemp, Warning, TEXT("슬라임 종류 개수 : %d"), PlaySaveGame->SaveFarmSlimes.Num());
	}
}

void USlimeGameInstance::SetFarmSlimeLocation(FName SlimeID, FVector LastLocation)
{
	for (auto& Slime : PlaySaveGame->SaveFarmSlimes)
	{
		if (Slime.SlimeID == SlimeID)
		{
			Slime.SlimeLastLocations.Add(LastLocation);
			
			if (Slime.SlimeLastLocations.Num() > Slime.Count)
			{
				UE_LOG(LogTemp, Error, TEXT("엉? 위치가 슬라임보다 많다?"));
			}
			
			UE_LOG(LogTemp, Error, TEXT("저장 됨! 슬라임개수: %d, 위치: %d"), Slime.Count, Slime.SlimeLastLocations.Num());
		}
	}
}

void USlimeGameInstance::ClearFarmSlimeLocation()
{
	// SlimeFarm Location 초기화
	for (auto& Slime : PlaySaveGame->SaveFarmSlimes)
	{
		Slime.SlimeLastLocations.Empty();
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
	OutCurLevel = CurStatLevel;
	
	UE_LOG(LogTemp, Warning, TEXT("레벨 업 완료: LV %d → LV %d"), CurStatLevel - 1, CurStatLevel);
	return true;
}
