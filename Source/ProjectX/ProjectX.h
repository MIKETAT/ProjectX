// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	// 0 None
	None		UMETA(DisplayName = "None"),
	// 1 Confirm
	Confirm		UMETA(DisplayName = "Confirm"),
	// 2 Cancel
	Cancel		UMETA(DisplayName = "Cancel"),
	// 3 Action  for test
	Action		UMETA(DisplayName = "Action")
};

PROJECTX_API DECLARE_LOG_CATEGORY_EXTERN(ProjectX, Log, All);