// Fill out your copyright notice in the Description page of Project Settings.


#include "_NZW/SlimePlayerController.h"
#include "EnhancedInputSubsystems.h"

ASlimePlayerController::ASlimePlayerController()
	: InputMappingContext(nullptr),
	  MoveAction(nullptr), 
	  JumpAction(nullptr), 
	  LookAction(nullptr), 
	  SprintAction(nullptr)
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
			}
		}
	}
	
}
