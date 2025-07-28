// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemDragVisual.generated.h"

class UImage;
class UBorder;

UCLASS()
class PROJECTX_API UItemDragVisual : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, Category = "Inventory", meta = (BindWidget))
	TObjectPtr<UImage> DragIcon;

	UPROPERTY(BlueprintReadWrite, Category = "Inventory", meta = (BindWidget))
	TObjectPtr<UBorder> DragBorder;
};
