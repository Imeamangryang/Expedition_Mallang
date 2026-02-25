#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "VacuumableInfo.generated.h"

class USlimeItemDefinition;

UENUM(BlueprintType)
enum class EVacuumableType : uint8
{
	None	UMETA(DisplayName = "None"),
	Slime UMETA(DisplayName = "Slime"),
	Food  UMETA(DisplayName = "Food"),
	Plort UMETA(DisplayName = "Plort")
};

UENUM(BlueprintType)
enum class EFoodType : uint8
{
	None				UMETA(DisplayName = "None"),
	Fruit				UMETA(DisplayName = "Fruit"),
	Vegetable		UMETA(DisplayName = "Vegetable"),
	Omnivorous	UMETA(DisplayName = "Omnivorous")
};

USTRUCT(BlueprintType)
struct FVacuumableInfo : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ID;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ItemName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EVacuumableType VacuumType = EVacuumableType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EFoodType FoodType = EFoodType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* ImageIcon = nullptr;
	
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// TObjectPtr<USlimeItemDefinition> ItemBase;
};
