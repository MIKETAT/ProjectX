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
	float ReachDistance{100.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb")
	float TraceCapsuleRadius{42.f};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb")
	float LedgeOffset{5.f};	// 从上至下的检测需要向墙内部延伸一段距离而不是在边缘进行检测

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb")
	float WallAngleThreshold{5.f};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb")
	FVector2f LedgeHeight{100.f, 200.f};	// 相对于CapsuleComponent Location的高度。

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb")
	float ClimbLedgeThreshold{150.f};
};


UCLASS(BlueprintType)
class PROJECTX_API UClimbSettings : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	FClimbTraceSetting TraceSetting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings | Climb")
	TObjectPtr<UAnimMontage> ClimbMontage;
};
