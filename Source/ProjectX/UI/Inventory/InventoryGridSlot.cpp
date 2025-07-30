#include "InventoryGridSlot.h"

#include "DragItemOperation.h"
#include "ItemDragVisual.h"
#include "XPlayerController.h"
#include "Component/InventoryComponent.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/MenuAnchor.h"
#include "Components/TextBlock.h"
#include "Items/InventoryStatics.h"
#include "Items/ItemDefinition.h"
#include "Items/InventoryHUDInterface.h"

// 暂时只按照Category 和 Guid 匹配物品类型
bool UInventoryGridSlot::CanMatchItemType(TSubclassOf<UItemDefinition> Definition) const
{
	if (!Definition)
	{
		return false;
	}
	FItemManifest ItemManifest = UInventoryStatics::GetItemManifest(Definition);
	return ItemManifest.Category == SlotItemManifest.Category && ItemManifest.Guid == SlotItemManifest.Guid;
}

// 当前Slot能否容纳指定的物品。 todo:考虑堆叠和物品数量
bool UInventoryGridSlot::CanAcceptItem(TSubclassOf<UItemDefinition> Definition) const
{
	if (SlotStatus == EGridSlotStatus::Occupied)
	{
		return true;
	}
	if (!CanMatchItemType(Definition))
	{
		return false;
	}
	return GetRemainingCapacity() > 0;
}

void UInventoryGridSlot::NativeConstruct()
{
	Super::NativeConstruct();
	OwningController = Cast<AXPlayerController>(GetOwningPlayer());
	ensure(OwningController);
	ensure(SlotMenuAnchor);
	SlotMenuAnchor->SetPlacement(MenuPlacement_MenuRight);
}

void UInventoryGridSlot::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	SlotBorder->SetBrushColor(BaseColor);
}

FReply UInventoryGridSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// Right Mouse Button Click
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (HasItem())
		{
			HideTooltip();
			if (!SlotMenuAnchor->IsOpen())
			{
				SlotMenuAnchor->Open(true);
			}
		}
		return FReply::Handled();
	}
	// Left Mouse Button Click
	else if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return CustomDetectDrag(InMouseEvent, this, EKeys::LeftMouseButton);
	}
	return FReply::Unhandled();
}

// handle tooltip
void UInventoryGridSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	SlotBorder->SetBrushColor(HoverColor);
	ToggleTooltip();
}

void UInventoryGridSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	SlotBorder->SetBrushColor(BaseColor);
	ToggleTooltip();
}

FReply UInventoryGridSlot::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Handled();
}

void UInventoryGridSlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	if (HasItem())
	{
		HideTooltip();
		// todo: DragVisual 是否可以复用
		UItemDragVisual* DragVisual = CreateWidget<UItemDragVisual>(this, ItemDragVisualClass);
		ensureMsgf(DragVisual, TEXT("Create ItemDragVisual failed!"));
		DragVisual->DragIcon->SetBrushFromTexture(SlotItemManifest.Icon);
		DragVisual->DragBorder->SetBrushColor(SlotBorder->GetBrushColor());

		UDragItemOperation* DragOperation = NewObject<UDragItemOperation>();
		DragOperation->DefaultDragVisual = DragVisual;
		DragOperation->Pivot = EDragPivot::MouseDown;
		DragOperation->DraggedSlotIndex = SlotIndex;
		
		OutOperation = DragOperation;
	} else
	{
		OutOperation = nullptr;
	}
}

bool UInventoryGridSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UDragItemOperation* DragOperation = Cast<UDragItemOperation>(InOperation);
	if (!IsValid(DragOperation))
	{
		return false;
	}
	int32 SourceIndex = DragOperation->DraggedSlotIndex;
	int32 TargetIndex = SlotIndex;
	IInventoryHUDInterface::Execute_UI_MoveInventoryItem(OwningController, SourceIndex, TargetIndex);
	return false;
}

void UInventoryGridSlot::NativeOnDragCancelled(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragCancelled(InDragDropEvent, InOperation);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Drag Camcelled!"));
	}
}

// 只有左键才能拖拽
FReply UInventoryGridSlot::CustomDetectDrag(const FPointerEvent& InMouseEvent, UWidget* WidgetDetectingDrag,
	FKey DragKey)
{
	if (InMouseEvent.GetEffectingButton() == DragKey)
	{
		FEventReply Reply;
		Reply.NativeReply = FReply::Handled();
		if (WidgetDetectingDrag)
		{
			TSharedPtr<SWidget> SlateWidgetDetectingDrag = WidgetDetectingDrag->GetCachedWidget();
			if (SlateWidgetDetectingDrag.IsValid())
			{
				Reply.NativeReply = Reply.NativeReply.DetectDrag(SlateWidgetDetectingDrag.ToSharedRef(), DragKey);
				return Reply.NativeReply;
			}
		}
	}
	return FReply::Unhandled();
}

// Tooltip
void UInventoryGridSlot::DisplayTooltip()
{
	if (IsValid(GetToolTip()))
	{
		GetToolTip()->SetVisibility(ESlateVisibility::Visible);	
	}
}

void UInventoryGridSlot::HideTooltip()
{
	if (IsValid(GetToolTip()))
	{
		GetToolTip()->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UInventoryGridSlot::ToggleTooltip()
{
	if (IsValid(GetToolTip()))
	{
		if (HasItem())
		{
			DisplayTooltip();		
		} else
		{
			HideTooltip();
		}
	}
}

// ~ End of Tooltip


// 该函数仅适用于当前格子已占用
int32 UInventoryGridSlot::GetRemainingCapacity() const
{
	if (SlotStatus != EGridSlotStatus::Occupied)
	{
		return -1; 
	}
	if (!SlotItemManifest.bStackable)
	{
		return 0;
	}
	return SlotItemManifest.MaxStackSize - StackCount;
}

void UInventoryGridSlot::AddItemToSlot(TSubclassOf<UItemDefinition> Definition, int32 Count)
{
	if (Definition == nullptr || Count <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UInventoryGridSlot::AddItemToSlot. Invalid Definition or Count"));
		return;
	}
	SetItemDefinition(Definition);
	SetSlotStatus(EGridSlotStatus::Occupied);
	SetSlotItemManifest(UInventoryStatics::GetItemManifest(Definition));
	SetStackCount(Count);
	UpdateGridSlot();
}

void UInventoryGridSlot::RemoveItemFroSlot()
{
	SetItemDefinition(nullptr);
	SetSlotStatus(EGridSlotStatus::UnOccupied);
	SetStackCount(0);
	UpdateGridSlot();
}

void UInventoryGridSlot::UpdateGridSlot()
{
	UpdateSlotTexture();
	UpdateSlotStackCountText();
}

void UInventoryGridSlot::UpdateSlotTexture()
{
	ensure(ItemDefinition);
	if (HasItem())
	{
		SlotIcon->SetBrushFromTexture(SlotItemManifest.Icon);
		SlotIcon->SetOpacity(1);
	} else
	{
		SlotIcon->SetBrushFromTexture(DefaultIcon);
	}
	
	
}

void UInventoryGridSlot::UpdateSlotStackCountText()
{
	if (HasItem())
	{
		ItemCountText->SetText(FText::AsNumber(StackCount));	
	} else
	{
		ItemCountText->SetText(FText::FromString(TEXT("")));
	}
	
}

bool UInventoryGridSlot::HasItem() const
{
	return ItemDefinition != nullptr && StackCount > 0;
}
