#include "Component/InventoryComponent.h"

#include "XPlayerController.h"
#include "Engine/ActorChannel.h"
#include "Items/InventoryStatics.h"
#include "Items/ItemDefinition.h"
#include "Items/ItemInstance.h"
#include "Net/UnrealNetwork.h"

// 计算对应全局索引
int32 UInventoryComponent::GetGlobalSlotIndex(EItemCategory Category, int32 Row, int32 Col) const
{
	return GetCategoryIndex(Category) * GetSlotNumsPerCategory() + Row * Columns + Col;
}

int32 UInventoryComponent::GetInventoryCapacity() const
{
	constexpr int32 CategoryNum = static_cast<int32>(EItemCategory::NUM);
	return GetSlotNumsPerCategory() * CategoryNum;
}


bool UInventoryComponent::IsSlotIndexValid(int32 Index) const
{
	return Index >= 0 && Index < GetInventoryCapacity();
}

bool UInventoryComponent::HasItemAtIndex(int32 Index) const
{
	if (!IsSlotIndexValid(Index))
	{
		return false;
	}
	return InventoryList.HasItemAtIndex(Index);
}


void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	OwningController = Cast<AXPlayerController>(GetOwner());
}

UInventoryComponent::UInventoryComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
{
	SetIsReplicatedByDefault(true);
	SetIsReplicated(true);
}

void UInventoryComponent::InitializeComponent()
{
	Super::InitializeComponent();
	//ConstructInventoryList();
}


void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent, InventoryList);
}

bool UInventoryComponent::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething =  Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	for (const auto& Entry : InventoryList.Items)
	{
		UItemInstance* Instance = Entry.ItemInstance;
		if (Instance && IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}
	return WroteSomething;
}

void UInventoryComponent::ReadyForReplication()
{
	Super::ReadyForReplication();
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FInventoryEntry& Entry : InventoryList.Items)
		{
			UItemInstance* Instance = Entry.ItemInstance;
			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

// SourceIndex and TargetIndex are globalIndex
void UInventoryComponent::Server_UI_MoveInventoryItem_Implementation(int32 SourceIndex, int32 TargetIndex)
{
	if (SourceIndex == TargetIndex)
	{
		return;
	}
	// todo: consider stack
	if (HasItemAtIndex(TargetIndex))		
	{
		SwapItems(SourceIndex, TargetIndex);			// Swap Item
	} else
	{	
		MoveItemToIndex(SourceIndex, TargetIndex);		// To Empty Slot
	}
}

UItemInstance* UInventoryComponent::AddItemDefinition(TSubclassOf<UItemDefinition> ItemDef, int32 Count)
{
	UItemInstance* Instance = nullptr;
	if (ItemDef)
	{
		// todo: should call AddItemAtIndex
		Instance = AddItemToInventory(ItemDef, Count);
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Instance)
		{
			AddReplicatedSubObject(Instance);
		}
	}
	return Instance;
}


UItemInstance* UInventoryComponent::AddItemToInventory(TSubclassOf<UItemDefinition> ItemDef, int32 Count)
{
	// find SlotIndex to insert/ find first empty slot
	int32 SlotIndex = FindFirstAcceptableSlot(ItemDef);
	if (IsSlotIndexValid(SlotIndex))
	{
		OccupiedSlots.Add(SlotIndex);		//
		return InventoryList.AddEntryToIndex(ItemDef, SlotIndex, Count);
	}
	return nullptr;
}

// todo: stackable
int32 UInventoryComponent::FindFirstAcceptableSlot(TSubclassOf<UItemDefinition> ItemDef)
{
	EItemCategory Category = UInventoryStatics::GetItemCategoryByDefinition(ItemDef);
	int32 Capacity = GetInventoryCapacity();
	FIntPoint IndexRange = UInventoryStatics::GetCategoryIndexRange(Category, Rows, Columns);
	for (int32 SlotIndex = IndexRange.X; SlotIndex < IndexRange.Y; SlotIndex++)
	{
		// ignore stackable for now
		// so just find first empty slot
		if (OccupiedSlots.Find(SlotIndex) == nullptr)
		{
			return SlotIndex;
		}
	}
	return -1;	// -1 means no available slot
}


void UInventoryComponent::MoveItemToIndex(int32 SourceIndex, int32 TargetIndex)
{
	InventoryList.MoveEntryToIndex(SourceIndex, TargetIndex);
}

void UInventoryComponent::RemoveItemAtIndex(int32 ItemIndex)
{
	InventoryList.RemoveEntryAtIndex(ItemIndex);
}

void UInventoryComponent::SwapItems(int32 SourceIndex, int32 TargetIndex)
{
	InventoryList.SwapEntry(SourceIndex, TargetIndex);
}

void UInventoryComponent::RemoveItemInstance(UItemInstance* ItemInstance)
{
	InventoryList.RemoveEntry(ItemInstance);
	if (ItemInstance && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}

TArray<UItemInstance*> UInventoryComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

// todo: 是否允许数组中有多个同类型的物品(理论上MaxCount够大的话不需要多个Entry)
int32 UInventoryComponent::GetItemCountByDefinition(TSubclassOf<UItemDefinition> ItemDef) const
{
	int32 ItemCount = 0;
	for (const FInventoryEntry& Entry : InventoryList.Items)
	{
		UItemInstance* Instance = Entry.ItemInstance;
		if (Instance && Instance->GetItemDefinition() == ItemDef)
		{
			ItemCount += Entry.ItemCount;
		}
	}
	return ItemCount;
}

void FInventoryEntry::Clear()
{
	ItemInstance = nullptr;
	ItemCount = 0;
	LastObservedCount = 0;
}

/**
 *	 FInventoryList Functions
 */
TArray<UItemInstance*> FInventoryList::GetAllItems() const
{
	TArray<UItemInstance*> AllItems;
	AllItems.Reserve(Items.Num());
	for (const auto& Entry : Items)
	{
		if (Entry.ItemInstance != nullptr)
		{
			AllItems.Add(Entry.ItemInstance);
		}
	}
	return AllItems;
}

void FInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FInventoryEntry& Entry = Items[Index];
		Entry.LastObservedCount = 0;
		OwnerInventoryComponent->OnItemRemoved.Broadcast(Entry, FinalSize);
	}
}

void FInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FInventoryEntry& Entry = Items[Index];
		Entry.LastObservedCount = Entry.ItemCount;
		OwnerInventoryComponent->OnItemAdded.Broadcast(Entry, FinalSize);
	}
}

void FInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FInventoryEntry& Entry = Items[Index];
		check(Entry.LastObservedCount != INDEX_NONE);
		Entry.LastObservedCount = Entry.ItemCount;
		OwnerInventoryComponent->OnItemChanged.Broadcast(Entry, FinalSize);
	}
}

UItemInstance* FInventoryList::AddEntryToIndex(TSubclassOf<UItemDefinition> ItemDef, int32 SlotIndex, int32 Count)
{
	UItemInstance* Instance = nullptr;
	check(ItemDef);
	FInventoryEntry Entry;
	Entry.ItemInstance = NewObject<UItemInstance>(OwnerInventoryComponent->GetOwner());
	Entry.ItemInstance->SetItemDefinition(ItemDef);
	Entry.ItemCount = Count;
	Entry.SlotIndex = SlotIndex;
	Instance = Entry.ItemInstance;
	MarkItemDirty(Items.Add_GetRef(MoveTemp(Entry)));
	return Instance;
}

// move item to empty slot
void FInventoryList::MoveEntryToIndex(int32 SourceIndex, int32 TargetIndex)
{
	for (auto EntryIt = Items.CreateIterator(); EntryIt; EntryIt++)
	{
		FInventoryEntry& SourceEntry = *EntryIt;
		if (SourceEntry.SlotIndex == SourceIndex)
		{
			FInventoryEntry TargetEntry = SourceEntry;
			TargetEntry.SlotIndex = TargetIndex;
			EntryIt.RemoveCurrent();
			MarkItemDirty(Items.Add_GetRef(MoveTemp(TargetEntry)));
			MarkArrayDirty();
			break;
		}
	}
}

void FInventoryList::SwapEntry(int32 SourceIndex, int32 TargetIndex)
{
	for (auto EntryIt = Items.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInventoryEntry& Entry = *EntryIt;
		if (Entry.SlotIndex == SourceIndex)
		{
			Entry.SlotIndex = TargetIndex;
			MarkItemDirty(Entry);
		} else if (Entry.SlotIndex == TargetIndex)
		{
			Entry.SlotIndex = SourceIndex;
			MarkItemDirty(Entry);
		}
	}
}

void FInventoryList::RemoveEntry(UItemInstance* ItemInstance)
{
	for (auto EntryIter = Items.CreateIterator(); EntryIter; ++EntryIter)
	{
		FInventoryEntry& Entry = *EntryIter;
		if (Entry.ItemInstance == ItemInstance)
		{
			EntryIter.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

void FInventoryList::RemoveEntryAtIndex(int32 Index)
{
	for (auto EntryIt = Items.CreateIterator(); EntryIt; ++EntryIt)
	{
		FInventoryEntry& Entry = *EntryIt;
		if (Entry.SlotIndex == Index)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
			break;
		}
	}
}

bool FInventoryList::HasItemAtIndex(int32 Index) const
{
	for (auto EntryIt = Items.CreateConstIterator(); EntryIt; ++EntryIt)
	{
		const  FInventoryEntry& Entry = *EntryIt;
		if (Entry.SlotIndex == Index)
		{
			return true;
		}
	}
	return false;
}
