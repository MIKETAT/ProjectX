// Fill out your copyright notice in the Description page of Project Settings.


#include "XAttributeSet.h"

#include "Net/UnrealNetwork.h"

UXAttributeSet::UXAttributeSet()
{
}

void UXAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UXAttributeSet, Health);
	DOREPLIFETIME(UXAttributeSet, MaxHealth);
	DOREPLIFETIME(UXAttributeSet, Stamina);
	DOREPLIFETIME(UXAttributeSet, MaxStamina);
}

void UXAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UXAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
}
