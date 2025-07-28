#pragma once

#include "InventoryTypes.generated.h"

// 物品类别 
UENUM(BlueprintType)
enum class EItemCategory : uint8
{
	Equipment UMETA(DisplayName = "Equipment"),
	Consumables UMETA(DisplayName = "Consumables"),
	NUM,
	UNSET UMETA(Hidden)
};
ENUM_RANGE_BY_COUNT(EItemCategory, EItemCategory::NUM);

USTRUCT(BlueprintType)
struct FItemQuality
{
	GENERATED_BODY()
	static FLinearColor Base;
	static FLinearColor Hover;
	static FLinearColor Common;
	static FLinearColor Rare;
	static FLinearColor Epic;
	static FLinearColor Legendary;
};
