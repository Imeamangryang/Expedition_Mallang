// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VacuumableInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UVacuumableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class EXPEDITION_MALLANG_API IVacuumableInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetID(FName NewID);
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	FName GetID();
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UClass* GetItemActorClass();
};
