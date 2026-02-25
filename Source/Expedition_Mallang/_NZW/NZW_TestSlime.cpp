// Fill out your copyright notice in the Description page of Project Settings.


#include "_NZW/NZW_TestSlime.h"

#include "Components/BoxComponent.h"

// Sets default values
ANZW_TestSlime::ANZW_TestSlime() : ID(TEXT(""))
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	SetRootComponent(BoxComp);
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(BoxComp);
	
	//. 외형 파일 직접 로드해서 적용
	ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	if (Cube.Succeeded()) MeshComp->SetStaticMesh(Cube.Object);
	
	ConstructorHelpers::FObjectFinder<UMaterial> CubeMat(TEXT("/Script/Engine.Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'"));
	if (CubeMat.Succeeded()) MeshComp->SetMaterial(0, CubeMat.Object);
}

// Called when the game starts or when spawned
void ANZW_TestSlime::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANZW_TestSlime::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FName ANZW_TestSlime::GetID_Implementation()
{
	return ID;
}

UClass* ANZW_TestSlime::GetItemActorClass_Implementation()
{
	return this->GetClass();
}

