// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryStatics.generated.h"

class UItemDefinition;
struct FItemManifest;
class UInventoryComponent;

/**
 * 有关Inventory的静态函数
 */

class UInventoryComponent;

UCLASS()
class PROJECTX_API UInventoryStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	static UInventoryComponent* GetInventoryComponent(const APlayerController* PlayerController);

	UFUNCTION()
	static int32 GetSlotIndexFromPosition(const FIntPoint & Position, int32 Columns);

	UFUNCTION()
	static FIntPoint GetPositionFromSlotIndex(int32 SlotIndex, int32 Columns);

	UFUNCTION(BlueprintPure)
	static FItemManifest GetItemManifest(TSubclassOf<UItemDefinition> ItemDef);

	UFUNCTION()
	static EItemCategory GetItemCategoryByDefinition(TSubclassOf<UItemDefinition> ItemDef);
	
	UFUNCTION()
	static EItemCategory GetItemCategoryBySlotIndex(int32 SlotIndex, int32 Rows, int32 Columns);
	
	UFUNCTION()
	static FSlotLocation GetSlotLocation(int32 GlobalIndex, int32 Rows, int32 Columns);

	UFUNCTION()
	static FIntPoint GetCategoryIndexRange(EItemCategory Category, int32 Rows, int32 Columns);
};
