// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/PlayerState.h"
#include "XPlayerState.generated.h"

class ACharacterBase;
struct FOnAttributeChangeData;
class AXPlayerController;
class UGameplayEffect;
class UXGameplayAbility;
class UXAttributeSet;
class UXAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStatusChangedEvent, float, NewValue, float, MaxValue);

UCLASS()
class PROJECTX_API AXPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	// Functions
public:
	AXPlayerState();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UXAttributeSet* GetAttributeSet() const;

	UFUNCTION(BlueprintPure)
	bool IsAlive() const { return GetHealth() > 0.f; }
	
	// Attribute Change Event in Blueprint
	
	void OnHealthAttributeChanged(const FOnAttributeChangeData& Data);
	void OnManaAttributeChanged(const FOnAttributeChangeData& Data);
	void OnStaminaAttributeChanged(const FOnAttributeChangeData& Data);
	void OnMaxWalkSpeedAttributeChanged(const FOnAttributeChangeData& Data);
	
	void BindUIRelativeDelegates();
	
	// Attribute Getter
	UFUNCTION(BlueprintCallable)
	virtual float GetHealth() const;		// I don't know why in actionRPG it's virtual

	UFUNCTION(BlueprintCallable)
	virtual float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable)
	virtual float GetMana() const;

	UFUNCTION(BlueprintCallable)
	virtual float GetMaxMana() const;
	
	UFUNCTION(BlueprintCallable)
	virtual float GetStamina() const;

	UFUNCTION(BlueprintCallable)
	virtual float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable)
	virtual float GetMaxWalkSpeed() const;
	
protected:
	virtual void BeginPlay() override;
private:

	// Variables
public:
	// Delegates
protected:
	// GAS
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= GAS, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UXAbilitySystemComponent> Asc;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= GAS, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UXAttributeSet> AttributeSet;

	UPROPERTY()
	TWeakObjectPtr<AXPlayerController> OwnerController;

	UPROPERTY()
	TObjectPtr<ACharacterBase> OwnerCharacter;
private:
};
