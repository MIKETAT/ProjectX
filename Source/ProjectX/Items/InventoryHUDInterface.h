// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InventoryTypes.h"
#include "UObject/Interface.h"
#include "InventoryHUDInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInventoryHUDInterface : public UInterface
{
	GENERATED_BODY()
};

class PROJECTX_API IInventoryHUDInterface
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Inventory")
	void UI_MoveInventoryItem(int32 SourceIndex, int32 TargetIndex);
};
