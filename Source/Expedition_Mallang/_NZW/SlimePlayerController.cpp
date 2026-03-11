// Fill out your copyright notice in the Description page of Project Settings.


#include "_NZW/SlimePlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "SlimePlayer.h"
#include "SlimePlayerStatUI.h"
#include "SlimePlayerSlotUI.h"
#include "SlimeShopUI.h"
#include "SlimeDeadUI.h"
#include "Blueprint/UserWidget.h"

ASlimePlayerController::ASlimePlayerController()
	: InputMappingContext(nullptr),
	  MoveAction(nullptr), 
	  JumpAction(nullptr), 
	  LookAction(nullptr), 
	  SprintAction(nullptr),
	  SpotLightAction(nullptr),
		VacuumAction(nullptr),
		FireAction(nullptr),
		WaveCannonAction(nullptr),
	  Num_1Action(nullptr), Num_2Action(nullptr), Num_3Action(nullptr), Num_4Action(nullptr),
		InteractAction(nullptr),
		EnterAction(nullptr),
		ExitAction(nullptr),
		ESCAction(nullptr)
{
}

void ASlimePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// GetLocalPlayer() = 현재 PlayerController가 관리하는 Local Player 반환
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		// 해당 로컬 플레이어에 부착된 Enhanced Input Subsystem을 반환
		// 이러면 AddMappingContext나 RemoveMappingContext등을 호출해 입력 매칭을 동적으로 제어 가능
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMappingContext)
			{
				// 주어진 IMC를 Subsystem에 추가해 입력 매핑 활성화
				Subsystem->AddMappingContext(InputMappingContext, 0);
				// UE_LOG(LogTemp, Warning, TEXT("input mapping 활성화 완"));
			}
		}
	}
	
	if (StatUIClass)
	{
		StatUIWidget = CreateWidget<USlimePlayerStatUI>(this, StatUIClass);
		if (StatUIWidget)
		{
			StatUIWidget->AddToViewport();
		}
	}
	
	if (SlotUIClass)
	{
		SlotUIWidget = CreateWidget<USlimePlayerSlotUI>(this, SlotUIClass);
		if (SlotUIWidget)
		{
			SlotUIWidget->AddToViewport();
		}
	}	
	
	if (ShopUIClass)
	{
		ShopUIWidget = CreateWidget<USlimeShopUI>(this, ShopUIClass);
		if (ShopUIWidget)
		{
			ShopUIWidget->AddToViewport();
		}
	}	
	
	if (DeadUIClass)
	{
		DeadUIWidget = CreateWidget<USlimeDeadUI>(this, DeadUIClass);
		if (DeadUIWidget)
		{
			DeadUIWidget->AddToViewport(10);
		}
	}
	
	// 2) Pawn 얻어서 연결
	ASlimePlayer* SlimePlayer = Cast<ASlimePlayer>(GetPawn());
	if (SlimePlayer)
	{
		StatUIWidget->SetUpSlimePlayer(SlimePlayer);
		SlotUIWidget->SetUpSlimePlayer(SlimePlayer);
		ShopUIWidget->SetUpSlimePlayer(SlimePlayer);
		DeadUIWidget->SetUpSlimePlayer(SlimePlayer);
	}
	
	ShopUIWidget->SetVisibility(ESlateVisibility::Collapsed);
}
