// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ClimbSettings.generated.h"

USTRUCT(BlueprintType)
struct FClimbTraceSetting
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb")
	TEnumAsByte<ECollisionChannel> ClimbTraceChannel{ECC_Visibility};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb")
	float ReachDistance{150.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb")
	float TraceCapsuleRadius{20.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb")
	float TargetLocationOffset{15.f};	// 从上至下的检测需要向墙内部延伸一段距离而不是在边缘进行检测
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb")
	FVector2f LedgeHeight{50.f, 250.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb")
	float ClimbLedgeThreshold{100.f};
};


UCLASS()
class PROJECTX_API UClimbSettings : public UDataAsset
{
	GENERATED_BODY()
public:
	FClimbTraceSetting TraceSetting;
};
