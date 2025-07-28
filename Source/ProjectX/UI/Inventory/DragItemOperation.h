// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "DragItemOperation.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTX_API UDragItemOperation : public UDragDropOperation
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 DraggedSlotIndex;

	// todo 是否需要存储Manifest等信息
};
