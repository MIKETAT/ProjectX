#include "XAttributeSet.h"
#include "Net/UnrealNetwork.h"

UXAttributeSet::UXAttributeSet()
	: Health(1.f)
	, MaxHealth(1.f)
	, Mana(1.f)
	, MaxMana(1.f)
	, Stamina(1.f)
	, MaxStamina(1.f)
{}

void UXAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UXAttributeSet, Health);
	DOREPLIFETIME(UXAttributeSet, MaxHealth);
	DOREPLIFETIME(UXAttributeSet, Mana);
	DOREPLIFETIME(UXAttributeSet, MaxMana);
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
