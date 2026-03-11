// Fill out your copyright notice in the Description page of Project Settings.

#include "_NZW/SlimePlayer.h"

#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpotLightComponent.h"
#include "EnhancedInputComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

#include "InteractableInterface.h"
#include "SlimeGameInstance.h"
#include "SlimePlayerController.h"
#include "SlimePlayerSlotUI.h"
#include "SlimePlayerStatUI.h"
#include "SlimePlaySaveGame.h"
#include "SlimeShopUI.h"
#include "SlimeVacpack.h"


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
	
	JetpackAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("JetpackAudioComp"));
	JetpackAudioComp->bAutoActivate = false;
	VacuumAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("VaccumAudioComp"));
	VacuumAudioComp->bAutoActivate = false;
}

// Called when the game starts or when spawned
void ASlimePlayer::BeginPlay()
{
	Super::BeginPlay();
	
	//. Vacpack 생성 및 부착
	if (SlimeVacpackClass != nullptr)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		
		SlimeVacpack = GetWorld()->SpawnActor<ASlimeVacpack>(SlimeVacpackClass,  FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);
		
		// attach the weapon actor
		SlimeVacpack->AttachToActor(this, AttachmentRule);
		
		// attach the weapon meshes
		SlimeVacpack->GetWeaponFirstMesh()->AttachToComponent(FirstSkeletalMesh, AttachmentRule, SlimePlayerWeaponSocket);
		SlimeVacpack->GetWeaponThirdMesh()->AttachToComponent(GetMesh(), AttachmentRule, SlimePlayerWeaponSocket);
		// SlimeVacpack->GetWeaponFirstMesh()->SetRelativeRotation(FRotator(0, 90, 0));
		// SlimeVacpack->GetWeaponThirdMesh()->SetRelativeRotation(FRotator(0, 90, 0));
		// SlimeVacpack->GetWeaponFirstMesh()->SetRelativeScale3D(FVector(.03f, .03f, .03f));
		// SlimeVacpack->GetWeaponThirdMesh()->SetRelativeScale3D(FVector(.03f, .03f, .03f));
		
		SlimeVacpack->GetWeaponFirstMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		SlimeVacpack->GetWeaponFirstMesh()->SetAnimation(nullptr);
	}
	
	// Stat 초기화
	GI = Cast<USlimeGameInstance>(GetWorld()->GetGameInstance());
	if (GI || GI->PlaySaveGame)
	{
		FPlayerStat LoadStat;
		GI->GetStatData(GI->PlaySaveGame->SaveLevel, LoadStat);
		CurLevel = LoadStat.Level;
		CurHP = MaxHP = LoadStat.MaxHP;
		CurMP = MaxMP = LoadStat.MaxMP;
		Newbucks = GI->PlaySaveGame->SaveNewbucks;
		CurPlayTime = GI->PlaySaveGame->SavePlayTime;
		
		SetActorLocation(GI->PlaySaveGame->SaveLastLocation);
	}
	
	CurHP = MaxHP;
	CurMP = MaxMP;
	
	CurrentSpeed = MoveSpeed;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	GetCharacterMovement()->JumpZVelocity = JumpPower;
	
	GetWorldTimerManager().SetTimer(PlayTimeTimerHandle, this, &ASlimePlayer::UpdatePlayTime, 1.0f, true);
	
	//. Jetpack & Vacuuum Sound
	JetpackAudioComp->SetSound(JetpackLoopSound);
	VacuumAudioComp->SetSound(VacuumLoopSound);
	
	//. Delegate 초기화 호출
	OnUpdateHPInPercent.Broadcast(CurHP, MaxHP);
	OnUpdateMPInPercent.Broadcast(CurMP, MaxMP);
	OnUpdateNewbucks.Broadcast(Newbucks);
	OnShopInteraction.Broadcast(CurLevel);	
	OnUpdatePlayTime.Broadcast(CurPlayTime);
}

// Called every frame
void ASlimePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!bIsDead)
	{
		if (GetActorLocation().Z < 900.0f)
		{
			CurHP = 0.0f;
		}
		
		if (CurHP <= 0.1f)
		{
			PlayerDead();
		}
	}
	
	//. Movement 물리적으로 처리하면 충돌처리가 이상해짐 Character에 있는 Movement 사용하는것이 더 좋음 
	// FVector Distance = GetActorForwardVector() * MoveInput.X + GetActorRightVector() * MoveInput.Y;
	// Distance.Normalize();
	// FVector Location = Distance * CurrentSpeed * DeltaTime;
	// SetActorLocation(GetActorLocation() + Location);
	
	//. 애니메이션에 넘겨주기 위한 Velocity
	// Velocity = FVector(Location.X, Location.Y, Location.Z) / DeltaTime;
	Velocity = GetCharacterMovement()->Velocity;
	
	//. 발소리
	FootStepHandle();
	
	//. 제트팩
	Jetpack(DeltaTime);
	
	//. MP 차오르는
	if (bIsMPDecreasing) return;
	FillMPStart(DeltaTime);
	FillMP(DeltaTime);
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
			
			if (PlayerController->FireAction)
			{
				EnhancedInput->BindAction(PlayerController->FireAction, ETriggerEvent::Started, this, &ASlimePlayer::Fire);
			}			
			
			if (PlayerController->WaveCannonAction)
			{
				EnhancedInput->BindAction(PlayerController->WaveCannonAction, ETriggerEvent::Started, this, &ASlimePlayer::WaveCannon);
				EnhancedInput->BindAction(PlayerController->WaveCannonAction, ETriggerEvent::Completed, this, &ASlimePlayer::WaveCannonEnd);
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
						
			if (PlayerController->InteractAction)
			{
				EnhancedInput->BindAction(PlayerController->InteractAction, ETriggerEvent::Started, this, &ASlimePlayer::Interact);
				EnhancedInput->BindAction(PlayerController->InteractAction, ETriggerEvent::Started, this, &ASlimePlayer::ShopInteract);
			}
			
			if (PlayerController->EnterAction)
			{
				EnhancedInput->BindAction(PlayerController->EnterAction, ETriggerEvent::Started, this, &ASlimePlayer::EnterFunc);                                                                                                               
			}
			
			if (PlayerController->ExitAction)
			{
				EnhancedInput->BindAction(PlayerController->ExitAction, ETriggerEvent::Started, this, &ASlimePlayer::ExitShop);                                                                                                               
			}
		}
	}
}

void ASlimePlayer::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	
	JetpackCurTime = 0.0f;
	
	UGameplayStatics::PlaySoundAtLocation(this, JumpEndSound, GetActorLocation());
	JetpackAudioComp->FadeOut(1.0f, 0.f);
}

void ASlimePlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	GI->PlaySaveGame->SaveLastLocation = GetActorLocation();
}

void ASlimePlayer::FillMPStart(float DeltaTime)
{
	if (CurMP >= MaxMP) return;
	
	// 시간이 흐르다가
	CurFillMpTime += DeltaTime;
	
	// 만약 [현재시간]이 [생성시간]을 초과하면
	if (CurFillMpTime > StartFillMpTime)
	{
		bIsFillingMp = true;
	}
}

void ASlimePlayer::FillMP(float DeltaTime)
{
	if (bIsFillingMp == false) return;
	
	CurMP = FMath::Min(CurMP + MPFillSpeed * DeltaTime, MaxMP);
	OnUpdateMPInPercent.Broadcast(CurMP, MaxMP);
	
	if (CurMP >= MaxMP)
	{
		bIsFillingMp = false;
		CurFillMpTime = 0.0f;
	}
}

void ASlimePlayer::Move(const FInputActionValue& Value)
{
	if (!GetController()) return;
	
	MoveInput = Value.Get<FVector2D>();
	
	const FRotator YawRotation(0.0f, GetControlRotation().Yaw, 0.0f);
	const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDir   = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// CharacterMovementComponent에 이동 입력 전달 (벽 충돌·경사면 자동 처리)
	AddMovementInput(ForwardDir, MoveInput.X);
	AddMovementInput(RightDir,   MoveInput.Y);
}

void ASlimePlayer::FootStepHandle()
{
	if (!GetCharacterMovement()->IsMovingOnGround()) return;
	
	FVector CurLocation = GetActorLocation();
	AccumulatedDistance += FVector::Dist(CurLocation, LastFootStepLocation);
	LastFootStepLocation = CurLocation;
	
	if (AccumulatedDistance >= FootStepDistance)
	{
		AccumulatedDistance = 0.0f;
		PlayRandomFootStep();
	}
}

void ASlimePlayer::PlayRandomFootStep()
{
	if (FootStepSounds.IsEmpty()) return;
	
	int32 RandomNum = FMath::RandRange(0, FootStepSounds.Num() - 1);
	UGameplayStatics::PlaySoundAtLocation(this, FootStepSounds[RandomNum], GetActorLocation());
}

void ASlimePlayer::StartJump(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		if (!GetCharacterMovement()->IsFalling())
		{
			Jump();
			UGameplayStatics::PlaySoundAtLocation(this, JumpStartSound, GetActorLocation());
		}
		else
		{
			if (CurMP > 0.0f)
			{
				bIsJetpackOn = true;
				JetpackAudioComp->FadeIn(0.3f, 1.0f);
			
				// 떨어지는 중일 때 => 관성 없앰
				FVector CurVelocity = GetCharacterMovement()->Velocity;
				if (CurVelocity.Z < 0.0f)
				{
					CurVelocity.Z /= 3.0f;
					GetCharacterMovement()->Velocity = CurVelocity;
				}
			
				CurrentJetpackAcceleration = JetpackAcceleration;
			}
		}
	}
	
	// UE_LOG(LogTemp, Warning, TEXT("%f"), CurrentJetpackAcceleration);
}

void ASlimePlayer::EndJump(const FInputActionValue& Value)
{
	StopJumping();
	
	bIsJetpackOn = false;
	bIsMPDecreasing = false;
	JetpackAudioComp->FadeOut(0.3f, 0.f);
	
	CurrentJetpackAcceleration = 0.0f;
	
	JetpackCurTime = 0.0f;
}

void ASlimePlayer::Jetpack(float DeltaTime)
{
	if (CurMP <= 0)
	{
		CurMP = 0.0f;
		bIsJetpackOn = false;
		bIsMPDecreasing = false;
		JetpackAudioComp->FadeOut(0.3f, 0.f);
	}
	
	if (GetCharacterMovement()->IsFalling())
	{
		if (bIsJetpackOn)
		{
			JetpackCurTime += DeltaTime;
		}
	}	

	if (bIsJetpackOn && JetpackCurTime >= JetpackStartTime)
	{
		FVector V = GetCharacterMovement()->Velocity;
		V.Z += CurrentJetpackAcceleration * DeltaTime;
		GetCharacterMovement()->Velocity = V;
		
		bIsMPDecreasing = true;
		CurFillMpTime = 0.0f;
		UpdateMP(JetpackLoseMPTime * DeltaTime);
	}
} 

void ASlimePlayer::Look(const FInputActionValue& Value)
{
	FVector2D LookInput = Value.Get<FVector2D>();
	
	AddControllerYawInput(LookInput.X * MouseMoveSpeed * GetWorld()->DeltaTimeSeconds);	
	AddControllerPitchInput(LookInput.Y * MouseMoveSpeed * GetWorld()->DeltaTimeSeconds);
}

void ASlimePlayer::Sprint(const FInputActionValue& Value)
{
	if (Value.Get<bool>() && CurMP > 0.0f)
	{
		CurrentSpeed = SprintSpeed;
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
		
		bIsMPDecreasing = true;
		CurFillMpTime = 0.0f;
		UpdateMP(SprintLoseMPTime * GetWorld()->DeltaTimeSeconds);
		OnUpdateMPInPercent.Broadcast(CurMP, MaxMP);
	}
	else
	{
		bIsMPDecreasing = false;
		CurrentSpeed = MoveSpeed;
		GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	}
}

void ASlimePlayer::FlashLight(const FInputActionValue& Value)
{
	bIsSpotLightOn = !bIsSpotLightOn;
	UGameplayStatics::PlaySoundAtLocation(this, FlashLightSound, GetActorLocation());

	// UE_LOG(LogTemp, Warning, TEXT("Flashlight: %s"), bIsSpotLightOn ? TEXT("ON") : TEXT("OFF"));
	
	SpotLight->SetIntensity(bIsSpotLightOn ? SpotLightIntensity : 0.0f);
}

void ASlimePlayer::VacuumStart(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		SlimeVacpack->bIsVacuuming = true;
		
		VacuumAudioComp->FadeIn(0.3f, 1.0f);
		// UE_LOG(LogTemp, Warning, TEXT("Vacuum Start!"));
	}
}

void ASlimePlayer::VacuumEnd(const FInputActionValue& Value)
{
	SlimeVacpack->StopVacuuming();
	
	VacuumAudioComp->FadeOut(0.3f, 0.0f);
	// UE_LOG(LogTemp, Warning, TEXT("Vacuum End!"));
}

void ASlimePlayer::Fire(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		SlimeVacpack->FireVacuumable();
	}
}

void ASlimePlayer::WaveCannon(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		if (CurMP < WaveCannonMP) return;

		SlimeVacpack->WaveCannon();
		UGameplayStatics::PlaySoundAtLocation(this, WaveCannonSound, GetActorLocation());
		
		bIsMPDecreasing = true;
		CurFillMpTime = 0.0f;
		UpdateMP(WaveCannonMP);
	}
}

void ASlimePlayer::WaveCannonEnd(const FInputActionValue& Value)
{
	bIsMPDecreasing = false;
}

void ASlimePlayer::Num1Func(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		SlimeVacpack->SelectSlot(0);
	}
}

void ASlimePlayer::Num2Func(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		SlimeVacpack->SelectSlot(1);
	}
}

void ASlimePlayer::Num3Func(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		SlimeVacpack->SelectSlot(2);
	}
}

void ASlimePlayer::Num4Func(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		SlimeVacpack->SelectSlot(3);
	}
}

void ASlimePlayer::Interact(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		FVector Start = GetActorLocation();
		float Radius = 250.0f;

		TArray<FOverlapResult> Overlaps;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(this);

		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);
		ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldDynamic);

		bool bHit = GetWorld()->OverlapMultiByObjectType(
			Overlaps,
			Start,
			FQuat::Identity,
			ObjectQueryParams,
			FCollisionShape::MakeSphere(Radius),
			QueryParams
		);
		
		DrawDebugSphere(GetWorld(), Start, Radius, 12, FColor::Green, false, 2.0f);

		if (!bHit)
		{
			return;
		}

		for (const FOverlapResult& Result : Overlaps)
		{
			AActor* HitActor = Result.GetActor();
			if (!HitActor)
			{
				continue;
			}

			// 인터페이스 체크
			if (HitActor->GetClass()->ImplementsInterface(UInteractableInterface::StaticClass()))
			{
				ASlimePlayerController* PC = Cast<ASlimePlayerController>(GetWorld()->GetFirstPlayerController());
				PC->SlotUIWidget->SetVisibility(ESlateVisibility::Hidden);
				IInteractableInterface::Execute_Interact(HitActor);
				UGameplayStatics::PlaySoundAtLocation(this, InteractSound, GetActorLocation());
				break;
			}
		}
	}
}

void ASlimePlayer::EnterFunc(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		// 레벨업 쇼핑 
		if (bShopping)
		{
			FPlayerStat NextPlayerStat;
			int32 NextLevel = CurLevel+1;
			
			if (!GI->GetStatData(NextLevel, NextPlayerStat))
			{
				// UE_LOG(LogTemp, Error, TEXT("다음 레벨 없음!!!"));
				return;
			}
			
			if (NextPlayerStat.Cost <= Newbucks)
			{
				UpdateNewbucks(-NextPlayerStat.Cost);
				UGameplayStatics::PlaySoundAtLocation(this, LevelUpSuccessSound, GetActorLocation());
				
				// 레벨업 적용
				GI->ApplyUpgrade(CurLevel, NextPlayerStat);
				OnShopInteraction.Broadcast(CurLevel);
				
				MaxHP = NextPlayerStat.MaxHP;
				CurHP = MaxHP;
				OnUpdateHPInPercent.Broadcast(CurHP, MaxHP);
				
				MaxMP = NextPlayerStat.MaxMP;
				CurMP = MaxMP;
				OnUpdateMPInPercent.Broadcast(CurMP, MaxMP);
				
				SlimeVacpack->SetWaveCannonForce(NextPlayerStat.WavePower);
				
				//! 레벨 업 했으면 게임 세이브
				GI->PlaySaveGame->SaveLevel = CurLevel; 
				GI->PlaySaveGame->SaveNewbucks = Newbucks;
				GI->SaveGame();
				
				
				// UE_LOG(LogTemp, Error, TEXT("레벨업 성공"));				
			}
			else
			{
				OnShopNotEnoughNewbucks.Broadcast();
				UGameplayStatics::PlaySoundAtLocation(this, LevelUpFailedSound, GetActorLocation());
				// UE_LOG(LogTemp, Error, TEXT("레벨업 실패!!!"));
			}
			
			return;
		}
		
		// 다이얼로그 Delegate
		OnInteract.Broadcast();
	}
}

void ASlimePlayer::ShopInteract(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{
		// UE_LOG(LogTemp, Warning, TEXT("Shop Interact 들어옴"));
		OnShopInteraction.Broadcast(CurLevel);
	}
}

void ASlimePlayer::ExitShop(const FInputActionValue& Value)
{
	if (Value.Get<bool>())
	{ 
		// UE_LOG(LogTemp, Warning, TEXT("!!!!!TAB 눌림 !!!!!%d"), bShopping);
		if (bShopping)
		{
			ASlimePlayerController* PC = Cast<ASlimePlayerController>(GetWorld()->GetFirstPlayerController());
			PC->ShopUIWidget->SetVisibility(ESlateVisibility::Hidden);
			PC->SlotUIWidget->SetVisibility(ESlateVisibility::Visible);
			bShopping = false;
		}
	}
}

void ASlimePlayer::ESC_Menu(const FInputActionValue& Value)
{
}

void ASlimePlayer::PlayerDead()
{
	// UE_LOG(LogTemp, Warning, TEXT("죽음"));
	bIsDead = true;
	OnDead.Broadcast();
	SlimeVacpack->ClearInventorySlot();	
}

void ASlimePlayer::PlayerRebirth()
{
	// UE_LOG(LogTemp, Warning, TEXT("환생"));
	UpdateHP(-MaxHP);
	
	// 패널티 (+ 플레이 시간)
	CurPlayTime += PenaltyTime * 3600;
	GI->PlaySaveGame->SavePlayTime = CurPlayTime;
	GI->SessionStartTime = GetWorld()->GetTimeSeconds();
	
	OnUpdatePlayTime.Broadcast(CurPlayTime);
	bIsDead = false;
}

void ASlimePlayer::UpdateNewbucks(int32 AddNewbucks)
{
	Newbucks += AddNewbucks;
	GI->PlaySaveGame->SaveNewbucks = Newbucks;
	OnUpdateNewbucks.Broadcast(Newbucks);
}

void ASlimePlayer::UpdateHP(float Hp)
{
	CurHP -= Hp;
	CurHP = FMath::Max(CurHP, 0.0f);
	OnUpdateHPInPercent.Broadcast(CurHP, MaxHP);
}

void ASlimePlayer::UpdateMP(float MP)
{
	CurMP -= MP;
	CurMP = FMath::Max(CurMP, 0.0f);
	OnUpdateMPInPercent.Broadcast(CurMP, MaxMP);
}

void ASlimePlayer::UpdatePlayTime()
{
	CurPlayTime += GetWorld()->GetTimeSeconds() - GI->SessionStartTime;
	GI->PlaySaveGame->SavePlayTime = CurPlayTime;
	GI->SessionStartTime = GetWorld()->GetTimeSeconds();
	
	OnUpdatePlayTime.Broadcast(CurPlayTime);
}
