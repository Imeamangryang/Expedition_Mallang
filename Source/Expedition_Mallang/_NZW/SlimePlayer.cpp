// Fill out your copyright notice in the Description page of Project Settings.

#include "_NZW/SlimePlayer.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "EnhancedInputComponent.h"
#include "SlimePlayerController.h"
#include "SlimeVacpack.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameSession.h"

// Sets default values
ASlimePlayer::ASlimePlayer()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	/* 
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.f);
	
	ConstructorHelpers::FObjectFinder<USkeletalMesh> Manny(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	
	GetMesh()->SetSkeletalMeshAsset(Manny.Object);
	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
		
	// GetMesh()->SetOnlyOwnerSee(true);
	// GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	
	// 실제 게임에서는 보이지 않는 1인칭 메쉬 생성
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMesh->SetSkeletalMeshAsset(Manny.Object);
	SkeletalMesh->SetupAttachment(GetMesh());
	SkeletalMesh->bOnlyOwnerSee = true;
	//SkeletalMesh->SetOnlyOwnerSee(true);
	SkeletalMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	SkeletalMesh->SetCollisionProfileName(FName("NoCollision"));
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(GetMesh(), FName("head"));
	Camera->SetRelativeLocationAndRotation(FVector(5.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	Camera->bUsePawnControlRotation = true;
	Camera->bEnableFirstPersonFieldOfView = true;
	Camera->bEnableFirstPersonScale = true;
	Camera->FirstPersonFieldOfView = 70.f; 
	Camera->FirstPersonScale = 0.6f;
	
	GetMesh()->bOwnerNoSee = true;
	// GetMesh()->SetOwnerNoSee(true);				// 플레이어 본인에게는 숨김 (3인칭 메쉬)
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;
	GetMesh()->bCastHiddenShadow = true;	// 숨겨진 상태에서도 그림자는 명시적으로 캐스팅 가능
	*/
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	ConstructorHelpers::FObjectFinder<USkeletalMesh> Manny(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	GetMesh()->SetSkeletalMeshAsset(Manny.Object);
	GetMesh()->SetRelativeLocationAndRotation(
		FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Skeletal Mesh"));
	
	FirstSkeletalMesh->SetSkeletalMeshAsset(Manny.Object);
	FirstSkeletalMesh->SetupAttachment(GetMesh());
	FirstSkeletalMesh->SetOnlyOwnerSee(true);
	FirstSkeletalMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstSkeletalMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	Camera->SetupAttachment(FirstSkeletalMesh, FName("head"));
	Camera->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	Camera->bUsePawnControlRotation = true;
	Camera->bEnableFirstPersonFieldOfView = true;
	Camera->bEnableFirstPersonScale = true;
	Camera->FirstPersonFieldOfView = 70.0f;
	Camera->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);
}

// Called when the game starts or when spawned
void ASlimePlayer::BeginPlay()
{
	Super::BeginPlay();

	CurHP = MaxHP;
	CurMp = MaxMp;
	
	CurrentSpeed = MoveSpeed;
}

void ASlimePlayer::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	// Weapon 생성 및 부착
	if (SlimeVacpack == nullptr)
	{
		////  이전 설정 (잘못됐지만 남겨둠)
		//// FActorSpawnParameters SpawnParams;
		//// SpawnParams.Owner = this;
		//// SpawnParams.Instigator = GetInstigator();
		////
		//// SlimeVacpack = GetWorld()->SpawnActor<ASlimeVacpack>(ASlimeVacpack::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		////
		//// if (SlimeVacpack)
		//// {
		//// 	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, false);
		//// 	SlimeVacpack->AttachToComponent(GetMesh(), AttachmentRule, SlimePlayerWeaponSocket);
		//// }
		
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		
		SlimeVacpack = GetWorld()->SpawnActor<ASlimeVacpack>(ASlimeVacpack::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	
		const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

		// attach the weapon actor
		SlimeVacpack->AttachToActor(this, AttachmentRule);

		// attach the weapon meshes
		SlimeVacpack->GetWeaponFirstMesh()->AttachToComponent(FirstSkeletalMesh, AttachmentRule, SlimePlayerWeaponSocket);
		SlimeVacpack->GetWeaponThirdMesh()->AttachToComponent(GetMesh(), AttachmentRule, SlimePlayerWeaponSocket);
	}
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
	// }
	// else
	// {
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
