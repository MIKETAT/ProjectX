// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/RootMotionSource.h"
#include "UObject/Object.h"
#include "RootMotionSource_Hang.generated.h"

USTRUCT()
struct PROJECTX_API FRootMotionSource_Hang : public FRootMotionSource
{
	GENERATED_BODY()
	// Functions
public:
	FRootMotionSource_Hang();
	
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;
	virtual FRootMotionSource* Clone() const override;
	virtual bool Matches(const FRootMotionSource* Other) const override;
	virtual UScriptStruct* GetScriptStruct() const override;
	virtual void AddReferencedObjects(class FReferenceCollector& Collector) override;
	virtual void PrepareRootMotion(float SimulationTime, float MovementTickTime, const ACharacter& Character, const UCharacterMovementComponent& MoveComponent) override;
	// Variables
public:
	UPROPERTY()
	TObjectPtr<UAnimMontage> Montage;

	UPROPERTY()
	TWeakObjectPtr<UPrimitiveComponent> TargetPrimitive;
	
	UPROPERTY()
	FVector StartWorldLocation;

	UPROPERTY()
	FRotator StartWorldRotation;

	UPROPERTY()
	FVector RelativeTargetLocation;

	UPROPERTY()
	FRotator RelativeTargetRotation;
};

template<>
struct TStructOpsTypeTraits<FRootMotionSource_Hang> : public TStructOpsTypeTraitsBase2<FRootMotionSource_Hang>
{
	enum
	{
		WithNetSerializer = true,
	};
};
