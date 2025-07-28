// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/InventoryTypes.h"
#include "Items/ItemDefinition.h"
#include "InventoryGridSlot.generated.h"

class UMenuAnchor;
class UInventoryComponent;
class UItemDragVisual;
class UTextBlock;
class UBorder;
class AXPlayerController;
struct FItemManifest;
class UItemDefinition;
class UImage;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGridSlotEvent, int32, Index, const FPointerEvent&, MouseEvent);

UENUM(BlueprintType)
enum class EGridSlotStatus : uint8
{
	UnOccupied UMETA(DisplayName = "Unoccupied"),
	Occupied UMETA(DisplayName = "Occupied"),
	Selected UMETA(DisplayName = "Selected"),
	Disabled UMETA(DisplayName = "Disabled"),
	None UMETA(DisplayName = "None")
};

UCLASS()
class PROJECTX_API UInventoryGridSlot : public UUserWidget
{
	GENERATED_BODY()
// Functions
public:
	// Setters and Getters
	EGridSlotStatus GetSlotStatus() const { return SlotStatus; }
	void SetSlotStatus(EGridSlotStatus Status) { SlotStatus = Status; }
	int32 GetSlotIndex() const { return SlotIndex; }
	void SetSlotIndex(int32 Index) { SlotIndex = Index; }
	void SetItemDefinition(TSubclassOf<UItemDefinition> Definition) { ItemDefinition = Definition; }
	void SetSlotItemManifest(const FItemManifest& Manifest) { SlotItemManifest = Manifest; }
	void SetStackCount(int32 Count) { StackCount = Count; }
	// End of Setters and Getters

	void UpdateGridSlot();
	void AddItemToSlot(TSubclassOf<UItemDefinition> Definition, int32 Count = 1);
	void RemoveItemFroSlot();
	bool HasItem() const;
	bool CanAcceptItem(TSubclassOf<UItemDefinition> Definition) const;
	
	UFUNCTION()
	void ToggleTooltip();
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	FReply CustomDetectDrag(const FPointerEvent& InMouseEvent, UWidget* WidgetDetectingDrag, FKey DragKey);

	UFUNCTION()
	void DisplayTooltip();
	UFUNCTION()
	void HideTooltip();
private:
	void UpdateSlotTexture();
	void UpdateSlotStackCountText();
	
	bool CanMatchItemType(TSubclassOf<UItemDefinition> Definition) const;
	int32 GetRemainingCapacity() const;
	
// Variables
public:
	FGridSlotEvent OnSlotClicked;
	FGridSlotEvent OnSlotHovered;
	FGridSlotEvent OnSlotUnhovered;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UItemDragVisual> ItemDragVisualClass;

protected:
	// Bind Widget
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> SlotIcon;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UBorder> SlotBorder;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemCountText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMenuAnchor> SlotMenuAnchor;
	// ~End of Bind Widget
	
	EGridSlotStatus SlotStatus{EGridSlotStatus::UnOccupied};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor BaseColor{FItemQuality::Base};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor HoverColor{FItemQuality::Hover};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<UTexture2D> DefaultIcon;
	
	// 当前存放物品的信息
	UPROPERTY()
	TSubclassOf<UItemDefinition> ItemDefinition;

	FItemManifest SlotItemManifest;
	
	int32 StackCount{INDEX_NONE};

	int32 SlotIndex{INDEX_NONE};		// 对应Entry

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<AXPlayerController> OwningController;

	UPROPERTY()
	TObjectPtr<UInventoryComponent> OwningInventoryComponent;
};
