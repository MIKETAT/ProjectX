#pragma once

#include "GameplayTagContainer.h"
#include "UObject/PrimaryAssetId.h"
#include "XGameTypes.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FItemSlot {
	GENERATED_BODY()

	FItemSlot() : SlotIndex(-1) {}
	FItemSlot(const FPrimaryAssetId& InItemSlot, int32 InSlotIndex)
		: ItemSlot(InItemSlot), SlotIndex(InSlotIndex) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	FPrimaryAssetId ItemSlot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	int32 SlotIndex;

	bool operator==(const FItemSlot& Other) const {
		return ItemSlot == Other.ItemSlot && SlotIndex == Other.SlotIndex;
	}

	bool operator!=(const FItemSlot& Other) const {
		return !(*this == Other);
	}

	/** Implement so it can be used in map/set */
	friend inline uint32 GetTypeHash(const FItemSlot& ItemSlot) {
		return HashCombine(GetTypeHash(ItemSlot.ItemSlot), GetTypeHash(ItemSlot.SlotIndex));
	}

	bool IsValid() const {
		return ItemSlot.IsValid() && SlotIndex >= 0;
	}
};

/** 物品模版定义， 静态配置 */
UCLASS(BlueprintType)
class PROJECTX_API UItemDefinition : public UPrimaryDataAsset {
	GENERATED_BODY()

	UItemDefinition()
		: ItemName(FText::GetEmpty())
		, MaxStackSize(0)
		, bIsUnique(false)
		, Tags() {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"), Category = Item)
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"), Category = Item)
	FSlateBrush ItemIcon;	// 物品图标	or Texture2D?
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"), Category = Item)
	int32 MaxStackSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"), Category = Item)
	FGameplayTagContainer Tags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"), Category = Item)
	bool bIsUnique;		// 是否唯一物品

	bool operator==(const UItemDefinition& Other) const {
		return ItemName.EqualTo(Other.ItemName) && MaxStackSize == Other.MaxStackSize &&
			Tags == Other.Tags && bIsUnique == Other.bIsUnique;
	}

	bool operator!=(const UItemDefinition& Other) const {
		return !(*this == Other);
	}

	bool IsValid() const {
		return !ItemName.IsEmpty() && MaxStackSize > 0;
	}
};

/** 物品实例定义，动态配置 */
UCLASS(BlueprintType)
class PROJECTX_API UItemInstance : public UObject {
	GENERATED_BODY()

	UItemInstance()
		:	ItemId(FGuid::NewGuid())
		,	Definition(nullptr) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"), Category = Item)
	FGuid ItemId;		// 物品实例ID

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"), Category = Item)
	UItemDefinition* Definition;	// 物品定义 可以为空

	//  TArray<UItemFragment*> RuntimeFragments  
	//  实例可持有不同于模版的行为
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	// RuntimeFragments;

	bool operator==(const UItemInstance& Other) const {
		return ItemId == Other.ItemId && Definition == Other.Definition;
	}
	
	bool operator!=(const UItemInstance& Other) const {
		return !(*this == Other);
	}

	bool IsValid() const {
		return ItemId.IsValid();
	}
};

/** 背包中的物品单元 */
USTRUCT(Blueprintable, BlueprintType)
struct FInventoryEntry {
	GENERATED_BODY()

	FInventoryEntry() : ItemInstance(nullptr), ItemLevel(0), ItemCount(0) {}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	UItemInstance* ItemInstance;	// 物品实例
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	FPrimaryAssetId ItemAssetId;	// 物品资产ID

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	int32 ItemLevel;	// 物品等级

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Item)
	int32 ItemCount;	// 物品数量
	
	bool operator==(const FInventoryEntry& Other) const {
		return ItemInstance == Other.ItemInstance && ItemCount == Other.ItemCount;
	}
	
	bool operator!=(const FInventoryEntry& Other) const {
		return !(*this == Other);
	}

	bool IsValid() const {
		return ItemInstance != nullptr && ItemCount > 0;
	}
};
