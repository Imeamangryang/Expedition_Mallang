// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SlimePlayer.generated.h"

// class USpringArmComponent;
class ASlimePlayerController;
class ASlimeVacpack;
class USkeletalMeshComponent;
class UCameraComponent;
class USpotLightComponent;

// Enhanced Input에서 액션값을 받을 때 사용하는 구조체
struct FInputActionValue;
struct FPlayerStat;

/*! Delegate */ 
//. HP, MP, Newbucks
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUpdateHP_D, float, Cur, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUpdateMP_D, float, Cur, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateNB_D, int32, Newbucks);
//. ItemInfo
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FItemInfo_D, FName, ID);
//. ItemSlot
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSelectSlot, int32, CurSlotNum, int32, PrevSlotNum);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FVacuumed_D, FName, ID, int32, Count, int32, SlotNum);
//. Interact
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FInteract_D);
//. Shop Interaction
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FShopInteraction_D, int32, Level);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FShopNotEnoughNewbucks);


UCLASS()
class EXPEDITION_MALLANG_API ASlimePlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASlimePlayer();

protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
/*! 함수 */	
	UFUNCTION(BlueprintCallable)	
	FVector GetCurrentVelocity() { return Velocity; }
	
	UFUNCTION()
	void Move(const FInputActionValue& Value);
	
	UFUNCTION()
	void StartJump(const FInputActionValue& Value);
	UFUNCTION()
	void EndJump(const FInputActionValue& Value);
	
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	
	UFUNCTION()
	void Sprint(const FInputActionValue& Value);
	
	UFUNCTION()
	void FlashLight(const FInputActionValue& Value);
	
	UFUNCTION()
	void VacuumStart(const FInputActionValue& Value);
	UFUNCTION()
	void VacuumEnd(const FInputActionValue& Value);
	
	UFUNCTION()
	void Fire(const FInputActionValue& Value);
	
	UFUNCTION()
	void WaveCannon(const FInputActionValue& Value);
	
	UFUNCTION()
	void Num1Func(const FInputActionValue& Value);
	UFUNCTION()
	void Num2Func(const FInputActionValue& Value);
	UFUNCTION()
	void Num3Func(const FInputActionValue& Value);
	UFUNCTION()
	void Num4Func(const FInputActionValue& Value);
		
	UFUNCTION()
	void Interact(const FInputActionValue& Value);
	
	UFUNCTION()
	void EnterFunc(const FInputActionValue& Value);
	
	UFUNCTION()
	void ShopInteract(const FInputActionValue& Value);
	
	UFUNCTION()
	void ExitShop(const FInputActionValue& Value);
	
	UFUNCTION(BlueprintCallable)
	void UpdateNewbucks(int32 AddNewbucks);
	UFUNCTION(BlueprintCallable)
	void UpdateHP(float HP);
	UFUNCTION(BlueprintCallable)
	void UpdateMP(float MP);

protected:
	void Jetpack(float DeltaTime);
	
	void FillMPStart(float DeltaTime);
	void FillMP(float DeltaTime);
	
/*! 변수 */
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlimePlayer|Components", meta=(AllowPrivateAccess="true"))
	USkeletalMeshComponent* FirstSkeletalMesh;	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlimePlayer|Components", meta=(AllowPrivateAccess="true"))
	UCameraComponent* Camera;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SlimePlayer|Components", meta=(AllowPrivateAccess="true"))
	USpotLightComponent* SpotLight;
	
	UPROPERTY()
	ASlimePlayerController* PC;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Stat")
	int32 CurLevel = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Stat")
	float MaxHP = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Stat")
	float CurHP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Stat")
	float MaxMP = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Stat")
	float CurMP;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Stat")
	int32 Newbucks = 10000;	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Stat")
	float SprintLoseMPTime = 5.0f;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Shop")
	bool bShopping = false;
	
	/** MP */
	bool bIsMPDecreasing = false;
	bool bIsFillingMp = false;
	float CurFillMpTime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Stat")
	float StartFillMpTime = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Stat")
	float MPFillSpeed = 100.0f;
	
	/** Movement Settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Movement")
	float MouseMoveSpeed = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Movement")
	FVector2D MoveInput;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Movement")
	float CurrentSpeed = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Movement")
	float MoveSpeed = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Movement")
	float SprintSpeed = 1000.f; 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Movement")
	FVector Velocity;
	
	/** Vacpack Settings */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SlimePlayer|Vacpack")
	ASlimeVacpack* SlimeVacpack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Vacpack")
	FName SlimePlayerWeaponSocket = FName("HandGrip_R");
	// int32 CurSelectSlot = -1;
	
	bool bWaveCannon = false;
	float WaveCannonCurTime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|WaveCannon", meta=(AllowPrivateAccess="true"))
	float WaveCannonCoolTime = 3.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vacpack|WaveCannon", meta=(AllowPrivateAccess="true"))
	float WaveCannonMP = 5.0f;
	
	/** SpotLight Settings */
	bool bIsSpotLightOn = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Equipment")
	float SpotLightIntensity = 1000.0f;
	
	/** JetPack */
	bool bIsJetpackOn = false;
	float CurrentJetpackAcceleration = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Equipment")
	float JetpackAcceleration = 1200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Equipment")
	float JetpackCurTime = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Equipment")
	float JetpackStartTime = 0.4f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer|Equipment")
	float JetpackLoseMPTime = 10.0f;

	/** Delegate Settings */
	UPROPERTY(BlueprintAssignable, Category="SlimePlayer|UI")
	FUpdateHP_D OnUpdateHPInPercent;
	UPROPERTY(BlueprintAssignable, Category="SlimePlayer|UI")
	FUpdateMP_D OnUpdateMPInPercent;
	UPROPERTY(BlueprintAssignable, Category="SlimePlayer|UI")
	FUpdateNB_D OnUpdateNewbucks;
	
	UPROPERTY(BlueprintAssignable, Category="SlimePlayer|UI")
	FSelectSlot OnSelectSlot;
	UPROPERTY(BlueprintAssignable, Category="SlimePlayer|UI")
	FItemInfo_D OnItemInfo;
	
	UPROPERTY(BlueprintAssignable, Category="SlimePlayer|Vaccum")
	FVacuumed_D OnVacuuming;
		
	UPROPERTY(BlueprintAssignable, Category="SlimePlayer|Interaction")
	FInteract_D OnInteract;
	
	UPROPERTY(BlueprintAssignable, Category="SlimePlayer|Shop")
	FShopInteraction_D OnShopInteraction;
	UPROPERTY(BlueprintAssignable, Category="SlimePlayer|Shop")
	FShopNotEnoughNewbucks OnShopNotEnoughNewbucks;
};
