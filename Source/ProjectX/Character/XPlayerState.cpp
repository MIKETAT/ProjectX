#include "XPlayerState.h"

#include "CharacterBase.h"
#include "ProjectX.h"
#include "XGameplayTags.h"
#include "XPlayerController.h"
#include "AbilitySystem/XAbilitySystemComponent.h"
#include "AbilitySystem/XAttributeSet.h"
#include "UI/HUD/PlayerHUD.h"

AXPlayerState::AXPlayerState()
{
	Asc = CreateDefaultSubobject<UXAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	Asc->SetIsReplicated(true);
	Asc->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UXAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AXPlayerState::GetAbilitySystemComponent() const
{
	return Asc;
}

UXAttributeSet* AXPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}

void AXPlayerState::OnHealthAttributeChanged(const FOnAttributeChangeData& Data)
{
	const float CurrentHealth = Data.NewValue;
	if (!OwnerController.IsValid())
	{
		OwnerController = Cast<AXPlayerController>(GetOwner());
	}
	if (OwnerController.IsValid() && OwnerController->GetPlayerHUD())
	{
		OwnerController->GetPlayerHUD()->SetCurrentHealth(CurrentHealth);
	}
}

void AXPlayerState::OnManaAttributeChanged(const FOnAttributeChangeData& Data)
{
	const float CurrentMana = Data.NewValue;
	if (!OwnerController.IsValid())
	{
		OwnerController = Cast<AXPlayerController>(GetOwner());
	}
	if (OwnerController.IsValid() && OwnerController->GetPlayerHUD())
	{
		OwnerController->GetPlayerHUD()->SetCurrentMana(CurrentMana);
	}
}

void AXPlayerState::OnStaminaAttributeChanged(const FOnAttributeChangeData& Data)
{
	const float CurrentStamina = Data.NewValue;
	if (!OwnerController.IsValid())
	{
		OwnerController = Cast<AXPlayerController>(GetOwner());
	}
	if (OwnerController.IsValid() && OwnerController->GetPlayerHUD())
	{
		OwnerController->GetPlayerHUD()->SetCurrentStamina(CurrentStamina);
	}
}

void AXPlayerState::OnMaxWalkSpeedAttributeChanged(const FOnAttributeChangeData& Data)
{
	
}

void AXPlayerState::BindUIRelativeDelegates()
{
	Asc->GetGameplayAttributeValueChangeDelegate(UXAttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthAttributeChanged);
	Asc->GetGameplayAttributeValueChangeDelegate(UXAttributeSet::GetManaAttribute()).AddUObject(this, &ThisClass::OnManaAttributeChanged);
	Asc->GetGameplayAttributeValueChangeDelegate(UXAttributeSet::GetStaminaAttribute()).AddUObject(this, &ThisClass::OnStaminaAttributeChanged);
	Asc->GetGameplayAttributeValueChangeDelegate(UXAttributeSet::GetMaxWalkSpeedAttribute()).AddUObject(this, &ThisClass::OnMaxWalkSpeedAttributeChanged);
}


float AXPlayerState::GetHealth() const
{
	if (!AttributeSet)		return 0.f;
	return AttributeSet->GetHealth();
}

float AXPlayerState::GetMaxHealth() const
{
	if (!AttributeSet)		return 0.f;
	return AttributeSet->GetMaxHealth();
}

float AXPlayerState::GetMana() const
{
	if (!AttributeSet)		return 0.f;
	return AttributeSet->GetMana();
}

float AXPlayerState::GetMaxMana() const
{
	if (!AttributeSet)		return 0.f;
	return AttributeSet->GetMaxMana();
}

float AXPlayerState::GetStamina() const
{
	if (!AttributeSet)		return 0.f;
	return AttributeSet->GetStamina();
}

float AXPlayerState::GetMaxStamina() const
{
	if (!AttributeSet)		return 0.f;
	return AttributeSet->GetMaxStamina();
}

float AXPlayerState::GetMaxWalkSpeed() const
{
	if (!AttributeSet)		return 0.f;
	return AttributeSet->GetMaxWalkSpeed();
}

void AXPlayerState::BeginPlay()
{
	Super::BeginPlay();
	OwnerController = Cast<AXPlayerController>(GetOwner());
	if (OwnerController.IsValid())
	{
		OwnerCharacter = Cast<ACharacterBase>(OwnerController->GetPawn());
	}
	if (!Asc || !OwnerController.IsValid())
	{
		return;
	}
	BindUIRelativeDelegates();
}
