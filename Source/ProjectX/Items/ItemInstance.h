#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "ItemInstance.generated.h"

class UItemDefinition;

UCLASS()
class PROJECTX_API UItemInstance : public UObject
{
	GENERATED_BODY()
public:
	UItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	TSubclassOf<UItemDefinition> GetItemDefinition() const { return ItemDef; }
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	
	void SetItemDefinition(TSubclassOf<UItemDefinition> Definition) { ItemDef = Definition; }
	
private:
	UPROPERTY(Replicated, EditAnywhere, Category= "Item", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UItemDefinition> ItemDef;

	UPROPERTY(Replicated)
	FGameplayTagContainer ItemTags;
};
