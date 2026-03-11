// Fill out your copyright notice in the Description page of Project Settings.


#include "_NZW/SlimeNPC.h"

#include "SlimePlayer.h"
#include "Components/CapsuleComponent.h"
#include "Misc/MapErrors.h"

// Sets default values
ASlimeNPC::ASlimeNPC()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>("CapsuleComp");
	SetRootComponent(CapsuleComp);
	
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMeshComp");
	SkeletalMeshComp->SetupAttachment(CapsuleComp);
}

// Called when the game starts or when spawned
void ASlimeNPC::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASlimeNPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASlimeNPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ASlimeNPC::Interact_Implementation()
{
	// UE_LOG(LogTemp, Warning, TEXT("Interact"));
		
	ASlimePlayer* Player = Cast<ASlimePlayer>(GetWorld()->GetFirstPlayerController()->GetPawn());
	Player->bShopping = true;
	
	bCanBeInteract = true;
}

