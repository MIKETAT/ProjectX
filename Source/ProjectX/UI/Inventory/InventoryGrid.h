// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Component/InventoryComponent.h"
#include "Items/InventoryTypes.h"
#include "InventoryGrid.generated.h"

struct FInventoryEntry;
class UInventoryComponent;
class UUniformGridPanel;
class UCanvasPanel;
class UInventoryGridSlot;


UCLASS()
class PROJECTX_API UInventoryGrid : public UUserWidget
{
	GENERATED_BODY()
// Functions
public:
	EItemCategory GetCategory() const { return GridCategory; }
	
	UFUNCTION()
	void HandleItemAdded(const FInventoryEntry& Entry, int32 FinalSize);

	UFUNCTION()
	void HandleItemRemoved(const FInventoryEntry& Entry, int32 FinalSize);
	
	UFUNCTION()
	void HandleItemChanged(const FInventoryEntry& Entry, int32 FinalSize);
	
	bool CanMatchCategory(EItemCategory ItemCategory) const { return GridCategory == ItemCategory; }

	UInventoryGridSlot* GetSlotWidgetBySlotIndex(int32 SlotIndex);
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	
private:
	void ConstructInventoryGrid();
// Variables
public:
	UPROPERTY()
	TObjectPtr<UInventoryComponent> InventoryComponent;

	UPROPERTY()
	TObjectPtr<UInventoryWidget> OwnerInventoryWidget;

	UPROPERTY()
	TMap<int32, UInventoryGridSlot*> SlotWidgetMap;		// map SlotIndex to SlotWidget
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UUniformGridPanel> UniformGridPanel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<UInventoryGridSlot> GridSlotClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Inventory")
	EItemCategory GridCategory;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	int32 Rows{INDEX_NONE};

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	int32 Columns{INDEX_NONE};

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	int32 GridSize{INDEX_NONE};
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	float TileSize{50.f};

};
