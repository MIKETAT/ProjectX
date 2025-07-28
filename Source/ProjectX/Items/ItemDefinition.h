#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "ItemDefinition.generated.h"

struct FItemQuality;
enum class EItemCategory : uint8;
class UItemInstance;

UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class PROJECTX_API UItemFragment : public UObject
{
	GENERATED_BODY()
public:
	virtual void OnInstanceCreated(UItemInstance* ItemInstance) const {};
};

// 
USTRUCT(BlueprintType)
struct PROJECTX_API FItemManifest
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FText DisplayName;
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FText Description;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	EItemCategory Category;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FGuid Guid;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	FLinearColor ItemQuality;

	// todo: 
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FItemQuality Quality;
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	bool bStackable{false};

	UPROPERTY(EditAnywhere, Category = "Inventory")
	int32 MaxStackSize{1};
};

/** 物品模版定义， 静态配置 */
UCLASS(BlueprintType, Blueprintable)
class PROJECTX_API UItemDefinition : public UObject {
	GENERATED_BODY()
public:
	UItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	const UItemFragment* FindFragmentByClass(TSubclassOf<UItemFragment> FragmentClass) const;
public:
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FItemManifest Manifest;
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TArray<TObjectPtr<UItemFragment>> Fragments;
};
