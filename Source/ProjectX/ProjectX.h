// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EAbilityInputID : uint8
{
	// 0 None
	None				UMETA(DisplayName = "None"),
	// 1 Confirm
	Confirm				UMETA(DisplayName = "Confirm"),
	// 2 Cancel
	Cancel				UMETA(DisplayName = "Cancel"),
	// 3 ToggleWalkRun
	ToggleWalkRun		UMETA(DisplayName = "ToggleWalkRun"),
	// 4 Sprint
	Sprint				UMETA(DisplayName = "Sprint")
};

PROJECTX_API DECLARE_LOG_CATEGORY_EXTERN(ProjectX, Log, All);


// Helper Macros

#if 1
inline float MacroDuration = 15.f;
#define XLOG(x) GEngine->AddOnScreenDebugMessage(-1, MacroDuration ? MacroDuration : -1.f, FColor::Red, x)
#define VLOG(x) UE_LOG(ProjectX, Error, x)
#define SLOG(x) UE_LOG(ProjectX, Error, TEXT("%s"), *FString(x))
#define POINT(x, c) DrawDebugPoint(GetWorld(), x, 15, c, false, MacroDuration ? MacroDuration : -1.f)
#define LINE(x, y, c) DrawDebugLine(GetWorld(), x, y, c, false, MacroDuration ? MacroDuration : -1.f)
#define CAPSULE(x, hh, r, c, time) DrawDebugCapsule(GetWorld(), x, hh, r, FQuat::Identity, c, false, time)

#else

#endif

