// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryGrid.h"

#include "InventoryGridSlot.h"
#include "Component/InventoryComponent.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Items/InventoryStatics.h"
#include "Items/ItemDefinition.h"
#include "Items/ItemInstance.h"

void UInventoryGrid::HandleItemAdded(const FInventoryEntry& Entry, int32 FinalSize)
{
	UE_LOG(LogTemp, Error, TEXT("HandleItemAdded"))
	EItemCategory Category = UInventoryStatics::GetItemCategoryBySlotIndex(Entry.SlotIndex, Rows, Columns);
	if (Category != GridCategory || !Entry.IsEntryValid())
	{
		return;
	}
	if (UInventoryGridSlot* SlotWidget = GetSlotWidgetBySlotIndex(Entry.SlotIndex))
	{
		TSubclassOf<UItemDefinition> ItemDef = Entry.ItemInstance->GetItemDefinition();
		SlotWidget->AddItemToSlot(ItemDef, Entry.ItemCount);
	}
}

void UInventoryGrid::HandleItemRemoved(const FInventoryEntry& Entry, int32 FinalSize)
{
	UE_LOG(LogTemp, Error, TEXT("HandleItemRemoved"));
	EItemCategory Category = UInventoryStatics::GetItemCategoryBySlotIndex(Entry.SlotIndex, Rows, Columns);
	if (Category != GridCategory || !Entry.IsEntryValid())
	{
		return;
	}
	if (UInventoryGridSlot* SlotWidget = GetSlotWidgetBySlotIndex(Entry.SlotIndex))
	{
		SlotWidget->RemoveItemFroSlot();
	}
}

void UInventoryGrid::HandleItemChanged(const FInventoryEntry& Entry, int32 FinalSize)
{
	UE_LOG(LogTemp, Error, TEXT("HandleItemChanged"));
	EItemCategory Category = UInventoryStatics::GetItemCategoryBySlotIndex(Entry.SlotIndex, Rows, Columns);
	if (Category != GridCategory || !Entry.IsEntryValid())
	{
		return;
	}
	if (UInventoryGridSlot* SlotWidget = GetSlotWidgetBySlotIndex(Entry.SlotIndex))
	{
		TSubclassOf<UItemDefinition> ItemDef = Entry.ItemInstance->GetItemDefinition();
		SlotWidget->AddItemToSlot(ItemDef, Entry.ItemCount);
	}
}

UInventoryGridSlot* UInventoryGrid::GetSlotWidgetBySlotIndex(int32 SlotIndex)
{
	if (UInventoryGridSlot** SlotPtr = SlotWidgetMap.Find(SlotIndex))
	{
		if (UInventoryGridSlot* SlotWidget = *SlotPtr)
		{
			return SlotWidget;
		}	
	}
	return nullptr;
}

void UInventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	InventoryComponent = UInventoryStatics::GetInventoryComponent(GetOwningPlayer());
	Rows = InventoryComponent->GetRows();
	Columns = InventoryComponent->GetColumns();
	GridSize = Rows * Columns;
	
	// bind FastArray delegate
	InventoryComponent->OnItemAdded.AddDynamic(this, &ThisClass::HandleItemAdded);
	InventoryComponent->OnItemRemoved.AddDynamic(this, &ThisClass::HandleItemRemoved);
	InventoryComponent->OnItemChanged.AddDynamic(this, &ThisClass::HandleItemChanged);
}

void UInventoryGrid::NativeConstruct()
{
	Super::NativeConstruct();
	//ConstructInventoryGrid();
}

void UInventoryGrid::ConstructInventoryGrid()
{
	ensureMsgf(Rows > 0 && Columns > 0, TEXT("Rows and Columns must be greater than 0!"));
	ensure(GridSlotClass && OwnerInventoryWidget);
	for (int32 i = 0; i < Rows; i++)
	{
		for (int32 j = 0; j < Columns; j++)
		{
			UInventoryGridSlot* ItemSlot = CreateWidget<UInventoryGridSlot>(this, GridSlotClass);
			ensure(ItemSlot && IsValid(InventoryComponent));
			int32 SlotIndex = InventoryComponent->GetGlobalSlotIndex(GridCategory, i, j);
			ItemSlot->SetSlotIndex(SlotIndex);
			SlotWidgetMap.Add(SlotIndex, ItemSlot);
			UUniformGridSlot* UniformSlot = UniformGridPanel->AddChildToUniformGrid(ItemSlot, i, j);
			ensure(UniformSlot);
			UniformSlot->SetHorizontalAlignment(HAlign_Fill);
			UniformSlot->SetVerticalAlignment(VAlign_Fill);
		}
	}
}
