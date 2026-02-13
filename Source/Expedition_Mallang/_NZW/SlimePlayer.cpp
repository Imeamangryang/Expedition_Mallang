// Fill out your copyright notice in the Description page of Project Settings.


#include "_NZW/SlimePlayer.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "EnhancedInputComponent.h"
#include "SkeletonTreeBuilder.h"
#include "SlimePlayerController.h"
#include "Camera/CameraComponent.h"
#include "Chaos/SoftsSpring.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
ASlimePlayer::ASlimePlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.f);
	
	ConstructorHelpers::FObjectFinder<USkeletalMesh> Manny(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	
	GetMesh()->SetSkeletalMeshAsset(Manny.Object);
	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
	
	// 실제 게임에서는 보이지 않는 1인칭 메쉬 생성
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetSkeletalMeshAsset(Manny.Object);
	SkeletalMesh->SetupAttachment(GetMesh());
	SkeletalMesh->SetOnlyOwnerSee(true);
	SkeletalMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	SkeletalMesh->SetCollisionProfileName(FName("NoCollision"));
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SkeletalMesh, FName("head"));
	Camera->SetRelativeLocationAndRotation(
		FVector(5.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	Camera->bUsePawnControlRotation = true;
	Camera->bEnableFirstPersonFieldOfView = true;
	Camera->bEnableFirstPersonScale = true;
	Camera->FirstPersonFieldOfView = 70.f; 
	Camera->FirstPersonScale = 0.6f;
	
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
	
	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);
}

// Called when the game starts or when spawned
void ASlimePlayer::BeginPlay()
{
	Super::BeginPlay();
	
	CurrentSpeed = MoveSpeed;
}

// Called every frame
void ASlimePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Movement
	FVector Distance = GetActorForwardVector() * MoveInput.X + GetActorRightVector() * MoveInput.Y;
	Distance.Normalize();
	FVector Location = Distance * CurrentSpeed * DeltaTime;
	SetActorLocation(GetActorLocation() + Location);
	
	Velocity = FVector(Location.X, Location.Y, Location.Z) / DeltaTime;
	UE_LOG(LogTemp, Warning, TEXT("Move Velocity: %f, %f, %f"), Velocity.X, Velocity.Y, Velocity.Z);
	// Jump
	// if (!GetCharacterMovement()->IsFalling())
	// {
	// 	
	// }
	// else
	// {
	// 	GetGravity
	// }
}

// Called to bind functionality to input
void ASlimePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ASlimePlayerController* PlayerController = Cast<ASlimePlayerController>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(PlayerController->MoveAction, ETriggerEvent::Triggered, this, &ASlimePlayer::Move);
				EnhancedInput->BindAction(PlayerController->MoveAction, ETriggerEvent::Completed, this, &ASlimePlayer::Move);
			}
			
			if (PlayerController->JumpAction)
			{
				EnhancedInput->BindAction(PlayerController->JumpAction, ETriggerEvent::Triggered, this, &ASlimePlayer::StartJump);
				EnhancedInput->BindAction(PlayerController->JumpAction, ETriggerEvent::Completed, this, &ASlimePlayer::EndJump);
			}
			
			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(PlayerController->LookAction, ETriggerEvent::Triggered, this, &ASlimePlayer::Look);
			}
			
			if (PlayerController->SprintAction)
			{
				EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Triggered, this, &ASlimePlayer::Sprint);
				EnhancedInput->BindAction(PlayerController->SprintAction, ETriggerEvent::Completed, this, &ASlimePlayer::Sprint);
			}
		}
	}
}

FVector ASlimePlayer::GetCurrentVelocity()
{
	return Velocity;
}

void ASlimePlayer::Move(const FInputActionValue& Value)
{
	if (!GetController()) return;
	
	MoveInput = Value.Get<FVector2D>();
}

void ASlimePlayer::StartJump(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		Jump();
	}
}

void ASlimePlayer::EndJump(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		StopJumping();
	}
}

void ASlimePlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookInput = Value.Get<FVector2D>();
	
	AddControllerYawInput(LookInput.X);	
	AddControllerPitchInput(LookInput.Y);
}

void ASlimePlayer::Sprint(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		CurrentSpeed = SprintSpeed;
	}
	else
	{
		CurrentSpeed = MoveSpeed;
	}
}
