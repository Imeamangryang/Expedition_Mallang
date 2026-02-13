// Fill out your copyright notice in the Description page of Project Settings.


#include "SlimeVacpack.h"

// Sets default values
ASlimeVacpack::ASlimeVacpack()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	WeaponSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponSkeletalMesh"));
	RootComponent = WeaponSkeletalMesh;
	
	ConstructorHelpers::FObjectFinder<USkeletalMesh> Weapon(TEXT("/Game/Weapons/GrenadeLauncher/Meshes/SKM_GrenadeLauncher.SKM_GrenadeLauncher"));
	if (Weapon.Succeeded()) WeaponSkeletalMesh->SetSkeletalMeshAsset(Weapon.Object);
	

	
	
	Muzzle = CreateDefaultSubobject<USceneComponent>("Muzzle");
	Muzzle->SetupAttachment(WeaponSkeletalMesh);
	Muzzle->SetRelativeLocation(FVector(0.0f, 63.0f, 11.0f));
}

// Called when the game starts or when spawned
void ASlimeVacpack::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASlimeVacpack::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

