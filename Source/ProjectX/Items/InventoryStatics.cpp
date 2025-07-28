// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryStatics.h"

#include "ItemDefinition.h"
#include "Component/InventoryComponent.h"

UInventoryComponent* UInventoryStatics::GetInventoryComponent(const APlayerController* PlayerController)
{
	if (!IsValid(PlayerController))
	{
		return nullptr;
	}
	return PlayerController->FindComponentByClass<UInventoryComponent>();
}

// Position(i, j)
int32 UInventoryStatics::GetSlotIndexFromPosition(const FIntPoint& Position, int32 Columns)
{
	return Position.Y + Position.X * Columns; 
}

FIntPoint UInventoryStatics::GetPositionFromSlotIndex(int32 SlotIndex, int32 Columns)
{
	return FIntPoint(SlotIndex % Columns, SlotIndex / Columns);
}

FItemManifest UInventoryStatics::GetItemManifest(TSubclassOf<UItemDefinition> ItemDef)
{
	return GetDefault<UItemDefinition>(ItemDef)->Manifest;
}

EItemCategory UInventoryStatics::GetItemCategoryByDefinition(TSubclassOf<UItemDefinition> ItemDef)
{
	return GetItemManifest(ItemDef).Category;
}

EItemCategory UInventoryStatics::GetItemCategoryBySlotIndex(int32 SlotIndex, int32 Rows, int32 Columns)
{
	int32 GridSize = Rows * Columns;
	return static_cast<EItemCategory>(SlotIndex / GridSize);
}

FSlotLocation UInventoryStatics::GetSlotLocation(int32 GlobalIndex, int32 Rows, int32 Columns)
{
	const int32 GridSize = Rows * Columns;
	const int32 LocalIndex = GlobalIndex % GridSize;	// 本页(本类别)索引
	return {
		static_cast<EItemCategory>(GlobalIndex / GridSize),
		LocalIndex
	};
}

FIntPoint UInventoryStatics::GetCategoryIndexRange(EItemCategory Category, int32 Rows, int32 Columns)
{
	int32 GridSize = Rows * Columns;
	int32 Begin = static_cast<int32>(Category) * GridSize;
	int32 End = (static_cast<int32>(Category) + 1) * GridSize;
	return FIntPoint(Begin, End);
}
