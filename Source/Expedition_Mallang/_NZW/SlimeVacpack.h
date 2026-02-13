// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SlimeVacpack.generated.h"

UCLASS()
class EXPEDITION_MALLANG_API ASlimeVacpack : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASlimeVacpack();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
/*! 함수 */
	UFUNCTION(BlueprintPure, Category="Weapon")
	USkeletalMeshComponent* GetWeaponSkeletalMesh() const { return WeaponSkeletalMesh; }
	
	
/*! 변수 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USkeletalMeshComponent* WeaponSkeletalMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	USceneComponent* Muzzle;
};
