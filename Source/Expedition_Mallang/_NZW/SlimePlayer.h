// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SlimePlayer.generated.h"

// class USpringArmComponent;
class UCameraComponent;
class USkeletalMeshComponent;
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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UFUNCTION(BlueprintCallable)	
	FVector GetCurrentVelocity();
	
protected:
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
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* SkeletalMesh;	
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer")
	FVector2D MoveInput;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer")
	float CurrentSpeed = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer")
	float MoveSpeed = 300.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer")
	float SprintSpeed = 500.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SlimePlayer")
	FVector Velocity;
	
};
