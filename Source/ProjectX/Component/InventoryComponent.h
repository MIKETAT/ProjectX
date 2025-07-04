#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "XGameTypes.h"
#include "InventoryComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTX_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemChanged, const FItemSlot&, Slot);

public:	
	UInventoryComponent();
	virtual void InitializeComponent() override;

	bool AddItem(const FPrimaryAssetId& ItemId, int32 Count = 1);
	bool RemoveItem(const FItemSlot& Slot, int32 Count = 1);
	bool UseItem(const FItemSlot& Slot, int32 Count = 1);

	
protected:
	virtual  void BeginPlay() override;

public:	
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
// Variables
public:
	UPROPERTY(BlueprintAssignable, Category = Item)
	FOnItemChanged OnItemAdded;	// 物品变更事件

	UPROPERTY(BlueprintAssignable, Category = Item)
	FOnItemChanged OnItemRemoved;	// 物品变更事件

	UPROPERTY(BlueprintAssignable, Category = Item)
	FOnItemChanged OnItemUsed;	// 物品使用事件
protected:
	UPROPERTY(VisibleAnywhere, Category = Item)
	TMap<FItemSlot, FInventoryEntry> InventoryMap;	// 背包物品映射

};
