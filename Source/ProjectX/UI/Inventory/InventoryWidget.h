// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "InventoryWidget.generated.h"

struct FInventoryEntry;
class UInventoryComponent;
class UInventoryGridSlot;
enum class EItemCategory : uint8;
class UButton;
class UWidgetSwitcher;
class UInventoryGrid;
/**
 * 
 */
UCLASS()
class PROJECTX_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void ShowEquipmentGrid();

	UFUNCTION()
	void ShowConsumablesGrid();

	void SetActiveGrid(UInventoryGrid* Grid, UButton* Button);
	void DisableButton(UButton* Button);
	//UInventoryGridSlot* GetInventoryGridSlot(EItemCategory Category, int32 LocalIndex) const;
	//UInventoryGridSlot* GetInventoryGridSlot(int32 GlobalIndex) const;

	void ToggleWindow();


protected:
	// Functions
	virtual void NativeOnInitialized() override;

public:

//	Variables
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> Switcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGrid> EquipmentGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UInventoryGrid> ConsumablesGrid;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Equippable;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Consumables;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> OwningInventoryComponent;

};
