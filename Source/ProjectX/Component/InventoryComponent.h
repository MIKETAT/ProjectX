#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemDefinition.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "InventoryComponent.generated.h"

class AXPlayerController;
class UInventoryComponent;
class UInventoryGridSlot;
class UInventoryStatics;
enum class EItemCategory : uint8;
class UInventoryWidget;
class UItemInstance;
class UItemDefinition;

USTRUCT(BlueprintType)
struct FSlotLocation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EItemCategory Category;

	UPROPERTY(BlueprintReadOnly)
	int32 LocalIndex;
};

// 记录单个SlotWidget存放物品的信息
USTRUCT()
struct FSlotStatus
{
	GENERATED_BODY()

	UPROPERTY()
	TSubclassOf<UItemDefinition> SlotItemDef;	// if not nullptr, SlotItemCount > 0
	
	UPROPERTY()
	int32 SlotItemCount{INDEX_NONE};

	bool operator==(const FSlotStatus& Other) const
	{
		return SlotItemDef == Other.SlotItemDef;
	}

	int32 GetRemainingCapacity() const
	{
		if (SlotItemDef)
		{
			FItemManifest Manifest = GetDefault<UItemDefinition>(SlotItemDef)->Manifest;
			return Manifest.bStackable ? FMath::Max(Manifest.MaxStackSize - SlotItemCount, 0) : 0;
		}
		return 0;
	}
};

/** 背包中的物品单元 */
USTRUCT()
struct FInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FInventoryEntry() : ItemInstance(nullptr), ItemCount(0), LastObservedCount(0) {}
	FInventoryEntry(UItemInstance* Instance): ItemInstance(Instance) {}
	FInventoryEntry(UItemInstance* Instance, int32 Count) : ItemInstance(Instance), ItemCount(Count) {}
	// copy constructor

	void Clear();
	bool IsEntryValid() const { return ItemInstance && ItemCount > 0; }
	
	UPROPERTY(EditAnywhere, Category = Item)
	TObjectPtr<UItemInstance> ItemInstance;	// 物品实例

	UPROPERTY(EditAnywhere, Category = Item)
	int32 ItemCount{INDEX_NONE};	// 物品数量

	UPROPERTY(NotReplicated)
	int32 LastObservedCount{INDEX_NONE};	// ItemCount的最新观察值

	UPROPERTY()
	int32 SlotIndex{INDEX_NONE};
};

USTRUCT()
struct FInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()
	FInventoryList() {}
	
	explicit FInventoryList(UInventoryComponent* InOwnerComponent) : OwnerInventoryComponent(InOwnerComponent) {}
	
	TArray<UItemInstance*> GetAllItems() const;
	
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract
	
	UItemInstance* AddEntryToIndex(TSubclassOf<UItemDefinition> ItemDef, int32 SlotIndex, int32 Count);
	UItemInstance* StackItemToIndex(int32 SlotIndex, int32 NewCount);
	void MoveEntryToIndex(int32 SourceIndex, int32 TargetIndex);
	void RemoveEntryAtIndex(int32 Index);
	void SwapEntry(int32 SourceIndex, int32 TargetIndex);
	void RemoveEntry(UItemInstance* ItemInstance);
	
	//bool IsIndexValid(int32 Index) const { return Index >= 0 && Index < Items.Num(); }
	bool HasItemAtIndex(int32 Index) const;

	UPROPERTY()
	TArray<FInventoryEntry> Items;

	UPROPERTY(NotReplicated)
	TObjectPtr<UInventoryComponent> OwnerInventoryComponent;

	// it's necessary
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryEntry, FInventoryList>(Items, DeltaParms, *this);
	}
};

template<>
struct TStructOpsTypeTraits<FInventoryList> : public TStructOpsTypeTraitsBase2<FInventoryList>
{
	enum 
	{
		WithNetDeltaSerializer = true
	};
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInventoryItemChanged, const FInventoryEntry&, Entry, int32, FinalSize);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FInventoryItemChanged, EItemCategory, Category, int32, LocalIndex);

UCLASS(BlueprintType, Blueprintable,  ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTX_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()
// Functions
public:	
	UInventoryComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	// UObject interface
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	// End of UObject interface

	// Inventory UI Function
	UFUNCTION(Server, Reliable)
	void Server_UI_MoveInventoryItem(int32 SourceIndex, int32 TargetIndex);
	
	// ~ End of Inventory UI Function
	
	/**
	*	1. 添加一个新物品到空格子	AddItemToInventory
		2. 添加堆叠物品到已有的格子(不存在拖拽物品进行堆叠, 加入背包时自动堆叠)
		3. 移除一个格子的物品		RemoveItemAtIndex
		4. 移动一个物品到空格子		MoveItemToIndex
		5. 交换两个格子的物品		SwapItems
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	UItemInstance* AddItemDefinition(TSubclassOf<UItemDefinition> ItemDef, int32 Count = 1);
	
	// add item to inventory. Stack or New entry
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	UItemInstance* AddItemToInventory(TSubclassOf<UItemDefinition> ItemDef, int32 Count);

	// Move an exist item to an empty index
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void MoveItemToIndex(int32 SourceIndex, int32 TargetIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Item")
	void RemoveItemInstance(UItemInstance* ItemInstance);
	
	// Swap Two Items that exists
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void SwapItems(int32 SourceIndex, int32 TargetIndex);
	
	// Remove Item at Index if it exists
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Inventory")
	void RemoveItemAtIndex(int32 ItemIndex);
	
	// find slot index. stack or new entry
	int32 FindFirstAcceptableSlot(TSubclassOf<UItemDefinition> ItemDef, int32 Count);

	UFUNCTION(BlueprintCallable, Category = "Item")
	TArray<UItemInstance*> GetAllItems() const;

	int32 GetItemCountByDefinition(TSubclassOf<UItemDefinition> ItemDef) const;
	
	AXPlayerController* GetOwningController() const { return OwningController; }

	int32 GetSlotNumsPerCategory() const { return Rows * Columns; }
	int32 GetCategoryIndex(EItemCategory Category) const { return static_cast<int32>(Category); }
	int32 GetGlobalSlotIndex(EItemCategory Category, int32 Row, int32 Col) const;
	int32 GetInventoryCapacity() const;
	bool IsSlotIndexValid(int32 Index) const;
	bool HasItemAtIndex(int32 Index) const;
	
	// Getter and Setter
	int32 GetRows() const { return Rows; }
	int32 GetColumns() const { return Columns; }
	// ~End of Getter and Setter
	
protected:
	virtual void BeginPlay() override;
	virtual void InitializeComponent() override;
// Variables
public:
	UPROPERTY()
	TMap<int32, FSlotStatus> OccupiedSlots;
	
	FInventoryItemChanged OnItemAdded;
	FInventoryItemChanged OnItemRemoved;
	FInventoryItemChanged OnItemChanged;
protected:
	UPROPERTY()
	TObjectPtr<AXPlayerController> OwningController;	
	
	UPROPERTY(Replicated)
	FInventoryList InventoryList;

	bool bInventoryOpen{false};		// 背包是否处于打开状态
	
	UPROPERTY(EditAnywhere, Category = "Inventory | Layout")
	int32 Rows{5};

	UPROPERTY(EditAnywhere, Category = "Inventory | Layout")
	int32 Columns{5};

	UPROPERTY(VisibleAnywhere, Category = "Inventory | Layout")
	int32 InventorySize{0};
};
