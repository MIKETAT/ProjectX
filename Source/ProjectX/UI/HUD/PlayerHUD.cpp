#include "PlayerHUD.h"

#include "Components/ProgressBar.h"

void UPlayerHUD::SetCurrentHealth(float InCurrentHealth)
{
	CurrentHealth = InCurrentHealth;
	UpdateHealth();
}

void UPlayerHUD::SetCurrentMana(float InCurrentMana)
{
	CurrentMana = InCurrentMana;
	UpdateMana();
}

void UPlayerHUD::SetCurrentStamina(float InCurrentStamina)
{
	CurrentStamina = InCurrentStamina;
	UpdateStamina();
}

void UPlayerHUD::SetMaxHealth(float InMaxHealth)
{
	MaxHealth = InMaxHealth;
}

void UPlayerHUD::SetMaxMana(float InMaxMana)
{
	MaxMana = InMaxMana;
}

void UPlayerHUD::SetMaxStamina(float InMaxStamina)
{
	MaxStamina = InMaxStamina;
}

void UPlayerHUD::UpdateHealth()
{
	float HealthPercentage = CurrentHealth / MaxHealth;	
	HealthPercentage = FMath::Clamp(HealthPercentage, 0.f, 1.f);
	if (HealthBar)
	{
		HealthBar->SetPercent(HealthPercentage);
	}
}
UE_ENABLE_OPTIMIZATION

void UPlayerHUD::UpdateMana()
{
	float ManaPercentage = CurrentMana / MaxMana;
	ManaPercentage = FMath::Clamp(ManaPercentage, 0.f, 1.f);
	if (ManaBar)
	{
		ManaBar->SetPercent(ManaPercentage);
	}
}

void UPlayerHUD::UpdateStamina()
{
	float StaminaPercentage = CurrentStamina / MaxStamina;
	StaminaPercentage = FMath::Clamp(StaminaPercentage, 0.f, 1.f);
	if (StaminaBar)
	{
		StaminaBar->SetPercent(StaminaPercentage);
		//UE_LOG(LogTemp, Error, TEXT("Set STM %f"), StaminaPercentage);
	}
}
