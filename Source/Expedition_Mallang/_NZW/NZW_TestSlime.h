// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VacuumableInterface.h"
#include "GameFramework/Actor.h"
#include "NZW_TestSlime.generated.h"

struct FVacuumableInfo;
class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class EXPEDITION_MALLANG_API ANZW_TestSlime : public AActor, public IVacuumableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANZW_TestSlime();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	//. 인터페이스 함수
	virtual void SetID_Implementation(FName NewID) override;
	virtual FName GetID_Implementation() override;
	virtual UClass* GetItemActorClass_Implementation() override;
	
//! 변수
	// 충돌체를 루트로 
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UBoxComponent> BoxComp;
	// 외형을 만들어 루트에 부착
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> MeshComp;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ID;
};
