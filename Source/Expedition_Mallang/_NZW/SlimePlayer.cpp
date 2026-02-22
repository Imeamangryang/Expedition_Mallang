// Fill out your copyright notice in the Description page of Project Settings.

#include "_NZW/SlimePlayer.h"

#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "EnhancedInputComponent.h"
#include "SlimePlayerController.h"
#include "SlimeVacpack.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameSession.h"

// Sets default values
ASlimePlayer::ASlimePlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	ConstructorHelpers::FObjectFinder<USkeletalMesh> Manny(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	if (Manny.Succeeded()) GetMesh()->SetSkeletalMeshAsset(Manny.Object);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, -88.0f), FRotator(0.0f, -90.0f, 0.0f));
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Skeletal Mesh"));
	
	if (Manny.Succeeded()) FirstSkeletalMesh->SetSkeletalMeshAsset(Manny.Object);
	// 1P: owner만 보이게
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
	// 3P: owner는 안 보이게 (남들에게만 보임)
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);
	
	//. 손전등
	SpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("SpotLight"));
	SpotLight->SetupAttachment(Camera);
	SpotLight->SetRelativeLocationAndRotation(FVector(30.0f, 17.5f, -5.0f), FRotator(-18.6f, -1.3f, 5.26f));
	SpotLight->Intensity = 0.5;
	SpotLight->SetIntensityUnits(ELightUnits::Lumens);
	SpotLight->SetIntensity(0);
	SpotLight->AttenuationRadius = 1050.0f;
	SpotLight->OuterConeAngle = 45.24f;
}

// Called when the game starts or when spawned
void ASlimePlayer::BeginPlay()
{
	Super::BeginPlay();
	
	//. Vacpack 생성 및 부착
	if (SlimeVacpack == nullptr)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		
		SlimeVacpack = GetWorld()->SpawnActor<ASlimeVacpack>(ASlimeVacpack::StaticClass(),  FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

		// attach the weapon actor
		SlimeVacpack->AttachToActor(this, AttachmentRule);

		// attach the weapon meshes
		SlimeVacpack->GetWeaponFirstMesh()->AttachToComponent(FirstSkeletalMesh, AttachmentRule, SlimePlayerWeaponSocket);
		SlimeVacpack->GetWeaponThirdMesh()->AttachToComponent(GetMesh(), AttachmentRule, SlimePlayerWeaponSocket);
		SlimeVacpack->GetWeaponFirstMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		SlimeVacpack->GetWeaponFirstMesh()->SetAnimation(nullptr);
	}
	
	//. Stat 초기화
	CurHP = MaxHP;
	CurMP = MaxMP;
	
	CurrentSpeed = MoveSpeed;
	
	//. Delegate 초기화 호출
	OnUpdateHPInPercent.Broadcast(CurHP/MaxHP);
	OnUpdateMPInPercent.Broadcast(CurMP/MaxMP);
	OnUpdateNewbucks.Broadcast(Newbucks);
}

// Called every frame
void ASlimePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	//. Movement
	FVector Distance = GetActorForwardVector() * MoveInput.X + GetActorRightVector() * MoveInput.Y;
	Distance.Normalize();
	FVector Location = Distance * CurrentSpeed * DeltaTime;
	SetActorLocation(GetActorLocation() + Location);
	
	//. 애니메이션에 넘겨주기 위한 Velocity
	Velocity = FVector(Location.X, Location.Y, Location.Z) / DeltaTime;

	//. 제트팩
	if (CurMP <= 0)
	{
		CurMP = 0.0f;
		bIsJetpackOn = false;
	}
	
	if (GetCharacterMovement()->IsFalling())
	{
		if (bIsJetpackOn)
		{
			JetpackCurTime += DeltaTime;
			UE_LOG(LogTemp, Warning, TEXT("JetpackCurTime : %f"), JetpackCurTime);
		}
	}
	else
	{
		JetpackCurTime = 0.0f;
	}

	if (bIsJetpackOn && JetpackCurTime >= JetpackStartTime)
	{
		FVector V = GetCharacterMovement()->Velocity;
		V.Z += CurrentJetpackAcceleration * DeltaTime;
		GetCharacterMovement()->Velocity = V;

		CurMP -= JetpackLoseMPTime * DeltaTime;
		OnUpdateMPInPercent.Broadcast(CurMP / 100.0f);
	}
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
				EnhancedInput->BindAction(PlayerController->JumpAction, ETriggerEvent::Started, this, &ASlimePlayer::StartJump);
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
			
			if (PlayerController->SpotLightAction)
			{
				EnhancedInput->BindAction(PlayerController->SpotLightAction, ETriggerEvent::Started, this, &ASlimePlayer::FlashLight);
			}
			
			if (PlayerController->VacuumAction)
			{
				EnhancedInput->BindAction(PlayerController->VacuumAction, ETriggerEvent::Started, this, &ASlimePlayer::VacuumStart);
				EnhancedInput->BindAction(PlayerController->VacuumAction, ETriggerEvent::Completed, this, &ASlimePlayer::VacuumEnd);
			}
			
			if (PlayerController->Num_1Action)
			{
				EnhancedInput->BindAction(PlayerController->Num_1Action, ETriggerEvent::Started, this, &ASlimePlayer::Num1Func);
			}
			
			if (PlayerController->Num_2Action)
			{
				EnhancedInput->BindAction(PlayerController->Num_2Action, ETriggerEvent::Started, this, &ASlimePlayer::Num2Func);
			}
			
			if (PlayerController->Num_3Action)
			{
				EnhancedInput->BindAction(PlayerController->Num_3Action, ETriggerEvent::Started, this, &ASlimePlayer::Num3Func);
			}
			
			if (PlayerController->Num_4Action)
			{
				EnhancedInput->BindAction(PlayerController->Num_4Action, ETriggerEvent::Started, this, &ASlimePlayer::Num4Func);
			}
		}
	}
}

void ASlimePlayer::Move(const FInputActionValue& Value)
{
	if (!GetController()) return;
	
	MoveInput = Value.Get<FVector2D>();
}

void ASlimePlayer::StartJump(const FInputActionValue& Value)
{
	if (Value.Get<bool>() && CurMP > 0.0f)
	{
		Jump();
		
		bIsJetpackOn = true;
		
		if (GetCharacterMovement()->IsFalling())
		{ // 공중에서 눌렀을 경우
			CurrentJetpackAcceleration = JetpackAcceleration + 500.0f;
		}
		else
		{ // 땅에서 점프 했을 경우
			CurrentJetpackAcceleration = JetpackAcceleration;
		}
	}
	
	UE_LOG(LogTemp, Warning, TEXT("%f"), CurrentJetpackAcceleration);
}

void ASlimePlayer::EndJump(const FInputActionValue& Value)
{
	StopJumping();
	bIsJetpackOn = false;
	CurrentJetpackAcceleration = 0.0f;
	
	JetpackCurTime = 0.0f;
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

void ASlimePlayer::FlashLight(const FInputActionValue& Value)
{
	bIsSpotLightOn = !bIsSpotLightOn;

	UE_LOG(LogTemp, Warning, TEXT("Flashlight: %s"), bIsSpotLightOn ? TEXT("ON") : TEXT("OFF"));
	
	SpotLight->SetIntensity(bIsSpotLightOn ? SpotLightIntensity : 0.0f);
}

void ASlimePlayer::VacuumStart(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		SlimeVacpack->bIsVacuuming = true;
		UE_LOG(LogTemp, Warning, TEXT("Vacuum Start!"));
	}
}

void ASlimePlayer::VacuumEnd(const FInputActionValue& Value)
{
	SlimeVacpack->StopVacuuming();
	UE_LOG(LogTemp, Warning, TEXT("Vacuum End!"));
}

void ASlimePlayer::Num1Func(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		CurHP -= 10.0f;
		OnUpdateHPInPercent.Broadcast(CurHP / 100.0f);
	}
}

void ASlimePlayer::Num2Func(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		CurHP += 10.0f;
		OnUpdateHPInPercent.Broadcast(CurHP / 100.0f);
	}
}

void ASlimePlayer::Num3Func(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		CurMP += 10.0f;
		OnUpdateMPInPercent.Broadcast(CurMP / 100.0f);
	}
}

void ASlimePlayer::Num4Func(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		Newbucks += 5.0f;
		OnUpdateNewbucks.Broadcast(Newbucks);
	}
}
