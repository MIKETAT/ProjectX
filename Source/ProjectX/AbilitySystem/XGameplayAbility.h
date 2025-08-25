// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "../ProjectX.h"
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "XGameplayAbility.generated.h"

UCLASS()
class PROJECTX_API UXGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UXGameplayAbility();

	// Input   ...no idea why I write it
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category= GAS)
	EAbilityInputID AbilityInputID = EAbilityInputID::None;

	// montage related to GA, it can be null
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> MontageToPlay;
};
