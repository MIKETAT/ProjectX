﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"
#include "InventoryGrid.h"
#include "XPlayerController.h"
#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "Items/InventoryStatics.h"

void UInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	/*CategoryToGrid.Add(EItemCategory::Equipment, EquipmentGrid);
	CategoryToGrid.Add(EItemCategory::Consumables, ConsumablesGrid);
	*/

	if (AXPlayerController* PC = Cast<AXPlayerController>(GetOwningPlayer()))
	{
		OwningInventoryComponent = UInventoryStatics::GetInventoryComponent(PC);
	}
	ensure(Btn_Equippable && Btn_Consumables);
	ensure(EquipmentGrid && ConsumablesGrid);

	EquipmentGrid->OwnerInventoryWidget = this;
	ConsumablesGrid->OwnerInventoryWidget = this;
	
	// bind button delegate 
	Btn_Equippable->OnClicked.AddDynamic(this, &ThisClass::ShowEquipmentGrid);
	Btn_Consumables->OnClicked.AddDynamic(this, &ThisClass::ShowConsumablesGrid);
	
	ShowEquipmentGrid();
}

void UInventoryWidget::SetActiveGrid(UInventoryGrid* Grid, UButton* Button)
{
	checkf(Switcher, TEXT("Switcher is not valid!"));
	ensureMsgf(Grid, TEXT("SetActiveGrid but Grid is Invalid"));
	DisableButton(Button);
	Switcher->SetActiveWidget(Grid);
}

// todo: do not hard code
void UInventoryWidget::ShowEquipmentGrid()
{
	SetActiveGrid(EquipmentGrid, Btn_Equippable);
}

// todo: do not hard code
void UInventoryWidget::ShowConsumablesGrid()
{
	SetActiveGrid(ConsumablesGrid, Btn_Consumables);
}

// todo: do not hard code button disabling
void UInventoryWidget::DisableButton(UButton* Button)
{
	Btn_Equippable->SetIsEnabled(true);
	Btn_Consumables->SetIsEnabled(true);
	Button->SetIsEnabled(false);
}

/*UInventoryGridSlot* UInventoryWidget::GetInventoryGridSlot(EItemCategory Category, int32 LocalIndex) const
{
	UInventoryGrid* Grid = CategoryToGrid[Category];
	if (!Grid)
	{
		UE_LOG(LogTemp, Error, TEXT("GetInventoryGridSlot Failed. Can't find Gird By Category %hhd"), Category);
		return nullptr;
	}
	return Grid->GetSlotByLocalIndex(LocalIndex);
}*/

/*UInventoryGridSlot* UInventoryWidget::GetInventoryGridSlot(int32 GlobalIndex) const
{
	ensureMsgf(OwningInventoryComponent, TEXT("UInventoryWidget::HandleItemAdded OwningInventoryComponent is nullptr"));
	FSlotLocation Location = UInventoryStatics::GetSlotLocation(GlobalIndex, OwningInventoryComponent->GetRows(), OwningInventoryComponent->GetColumns());
	return GetInventoryGridSlot(Location.Category, Location.LocalIndex);
}*/

void UInventoryWidget::ToggleWindow()
{
	if (GetVisibility() == ESlateVisibility::Hidden)
	{
		SetVisibility(ESlateVisibility::Visible);
	} else
	{
		SetVisibility(ESlateVisibility::Hidden);
	}
}
