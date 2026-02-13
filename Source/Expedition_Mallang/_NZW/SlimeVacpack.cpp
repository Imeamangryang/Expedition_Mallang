// Fill out your copyright notice in the Description page of Project Settings.


#include "SlimeVacpack.h"

// Sets default values
ASlimeVacpack::ASlimeVacpack()
{
	PrimaryActorTick.bCanEverTick = true;
	
	//. 루트 생성
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	//. 1인칭 무기 & 3인칭 무기 생섣ㅇ
	WeaponFirst = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon First"));
	WeaponThird = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Third"));
	
	//. 무기 메쉬 입히기 (여기는 Skeletal Mesh)
	ConstructorHelpers::FObjectFinder<USkeletalMesh> Weapon(TEXT("/Game/Weapons/GrenadeLauncher/Meshes/SKM_GrenadeLauncher.SKM_GrenadeLauncher"));
	if (Weapon.Succeeded()) 
	{
		WeaponFirst->SetSkeletalMeshAsset(Weapon.Object);
		WeaponThird->SetSkeletalMeshAsset(Weapon.Object);
	}
	
	//. 각각 맞는 설정으로 초기화
	WeaponFirst->SetupAttachment(RootComponent);
	WeaponFirst->SetCollisionProfileName(FName("NoCollision"));
	WeaponFirst->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
	WeaponFirst->bOnlyOwnerSee = true;
	
	WeaponThird->SetupAttachment(RootComponent);
	WeaponThird->SetCollisionProfileName(FName("NoCollision"));
	WeaponThird->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::WorldSpaceRepresentation);
	WeaponThird->bOwnerNoSee = true;
	
	//. 총구 설정
	Muzzle = CreateDefaultSubobject<USceneComponent>("Muzzle");
	Muzzle->SetupAttachment(WeaponFirst);
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

