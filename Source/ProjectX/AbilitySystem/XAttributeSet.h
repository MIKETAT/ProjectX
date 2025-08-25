#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "XAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
		GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAttributeChangeEvent, UAttributeSet*, AttributeSet, float, OldValue, float, NewValue);

UCLASS()
class PROJECTX_API UXAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	UXAttributeSet();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", Replicated);
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UXAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", Replicated);
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UXAttributeSet, MaxHealth);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", Replicated);
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS(UXAttributeSet, Mana);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", Replicated);
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS(UXAttributeSet, MaxMana);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", Replicated);
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS(UXAttributeSet, Stamina);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", Replicated);
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS(UXAttributeSet, MaxStamina);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", Replicated);
	FGameplayAttributeData MaxWalkSpeed;
	ATTRIBUTE_ACCESSORS(UXAttributeSet, MaxWalkSpeed);
};
