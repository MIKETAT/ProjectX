// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUD.generated.h"

class UXAttributeSet;
class UProgressBar;
UCLASS()
class PROJECTX_API UPlayerHUD : public UUserWidget
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable)
	void UpdateHealth();

	UFUNCTION(BlueprintCallable)
	void UpdateMana();

	UFUNCTION(BlueprintCallable)
	void UpdateStamina();
	
	// Setter and Getter
	void SetCurrentHealth(float InCurrentHealth);
	void SetCurrentMana(float InCurrentMana);
	void SetCurrentStamina(float InCurrentStamina);
	void SetMaxHealth(float InMaxHealth);
	void SetMaxMana(float InMaxMana);
	void SetMaxStamina(float InMaxStamina);

	
	// Variables
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UXAttributeSet> AttributeSet;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ManaBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> StaminaBar;
	
	// Attributes
	UPROPERTY(BlueprintReadWrite, Category = "Attributes")
	float CurrentHealth;

	UPROPERTY(BlueprintReadWrite, Category = "Attributes")
	float MaxHealth;

	UPROPERTY(BlueprintReadWrite, Category = "Attributes")
	float CurrentMana;

	UPROPERTY(BlueprintReadWrite, Category = "Attributes")
	float MaxMana;

	UPROPERTY(BlueprintReadWrite, Category = "Attributes")
	float CurrentStamina;

	UPROPERTY(BlueprintReadWrite, Category = "Attributes")
	float MaxStamina;
};
