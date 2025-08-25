// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UProjectXUtils.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTX_API UProjectXUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static FQuat GetTwist(const FQuat& Quaternion, const FVector& TwistAxis);
	
};

// todo
inline FQuat UProjectXUtils::GetTwist(const FQuat& Quaternion, const FVector& TwistAxis)
{
	const auto Projection{(TwistAxis | FVector{Quaternion.X, Quaternion.Y, Quaternion.Z}) * TwistAxis};

	return FQuat{Projection.X, Projection.Y, Projection.Z, Quaternion.W}.GetNormalized();
}
