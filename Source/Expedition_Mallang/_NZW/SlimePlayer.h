// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SlimePlayer.generated.h"

// class USpringArmComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class ASlimeVacpack;

// Enhanced Input에서 액션값을 받을 때 사용하는 구조체
struct FInputActionValue;


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
	
	virtual void OnConstruction(const FTransform& Transform) override;
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	
/*! 함수 */	
	UFUNCTION(BlueprintCallable)	
	FVector GetCurrentVelocity();
	
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
	
/*! 변수 */
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstSkeletalMesh;	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;
		
public:
	/** Stat Settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer.Stat")
	float MaxHP = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer.Stat")
	float CurHP;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer.Stat")
	float MaxMp = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer.Stat")
	float CurMp;	
	
	/** Movement Settings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer.Movement")
	FVector2D MoveInput;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer.Movement")
	float CurrentSpeed = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer.Movement")
	float MoveSpeed = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer.Movement")
	float SprintSpeed = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer.Movement")
	FVector Velocity;
	
	/** Weapon Settings */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SlimePlayer.Weapon")
	ASlimeVacpack* SlimeVacpack;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer.Weapon")
	FName SlimePlayerWeaponSocket = FName("HandGrip_R");
	
};
