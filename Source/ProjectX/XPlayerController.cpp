#include "XPlayerController.h"

#include "AbilitySystemComponent.h"
#include "UI/HUD/PlayerHUD.h"
#include "Blueprint/UserWidget.h"
#include "Character/CharacterBase.h"
#include "Character/XPlayerState.h"
#include "Component/InventoryComponent.h"
#include "Items/ItemBase.h"
#include "UI/Inventory/InventoryGrid.h"
#include "UI/Inventory/InventoryWidget.h"

void AXPlayerController::RemoveItemTest_Implementation(int32 Index)
{
	ensure(InventoryComp);
	InventoryComp->RemoveItemAtIndex(Index);
}

void AXPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ConstructInventoryWidget();
	//CreateHUD();
}

AXPlayerController::AXPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {
	InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	bReplicates = true;
}

// Server only
void AXPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	/*AXPlayerState* PS = GetPlayerState<AXPlayerState>();
	if (PS && PS->GetAbilitySystemComponent())
	{
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, InPawn);
	}*/
}

void AXPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
	OwnerCharacter = Cast<ACharacterBase>(P);
}

void AXPlayerController::ToggleInventory()
{
	if (!IsValid(InventoryWidget))
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryWidget is not valid!"));
		ConstructGridWidget();
		return;
	}
	ConstructGridWidget();
	InventoryWidget->ToggleWindow();
	SetInputModeByWidgetVisibility();
	// todo: broadcast event
}

UPlayerHUD* AXPlayerController::GetPlayerHUD()
{
	return PlayerHUD;
}


void AXPlayerController::ShowHUD(bool Visible)
{
	ensure(PlayerHUD);
	if (Visible)
	{
		PlayerHUD->SetVisibility(ESlateVisibility::Visible);
	} else
	{
		PlayerHUD->SetVisibility(ESlateVisibility::Hidden);
	}
}

// IInventoryHUDInterface
void AXPlayerController::UI_MoveInventoryItem_Implementation(int32 SourceIndex, int32 TargetIndex)
{
	if (!IsLocalController())
	{
		return;
	}
	if (HasAuthority())
	{
		InventoryComp->MoveItemToIndex(SourceIndex, SourceIndex);
	} else
	{
		InventoryComp->Server_UI_MoveInventoryItem(SourceIndex, TargetIndex);		
	}
}

void AXPlayerController::SetInputModeByWidgetVisibility()
{
	if (!IsValid(InventoryWidget))
	{
		return;
	}
	if (InventoryWidget->IsVisible())
	{
		SetInputMode(FInputModeGameAndUI());
		SetShowMouseCursor(true);
	} else
	{
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
	}
}

void AXPlayerController::PickupItem_Implementation(AItemBase* Item)
{
	if (InventoryComp && Item)
	{
		InventoryComp->AddItemDefinition(Item->GetItemDefinition());
	}
}
// ~ End of IInventoryHUDInterface


void AXPlayerController::ConstructInventoryWidget()
{
	if (!IsLocalController() || HasAuthority() || !IsValid(InventoryWidgetClass)) {
		return;
	}
	InventoryWidget = CreateWidget<UInventoryWidget>(this, InventoryWidgetClass);
	if (!InventoryWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("ConstructInventoryWidget::InventoryWidget is nullptr"));
		return;
	}
	InventoryWidget->SetVisibility(ESlateVisibility::Hidden);
	InventoryWidget->AddToViewport();
}

void AXPlayerController::ConstructGridWidget()
{
	ensure(InventoryWidget);
	ensure(InventoryWidget->EquipmentGrid && InventoryWidget->ConsumablesGrid);
	InventoryWidget->EquipmentGrid->ConstructInventoryGrid();
	InventoryWidget->ConsumablesGrid->ConstructInventoryGrid();
}

void AXPlayerController::CreateHUD()
{
	if (PlayerHUD)
	{
		ShowHUD(true);
		return;
	} 
	if (!HUDClass || !IsLocalController())
	{
		UE_LOG(LogTemp, Error, TEXT("CreateHUD::HUDClass is nullptr or not a local controller"));
		return;
	}
	AXPlayerState* PS = GetPlayerState<AXPlayerState>();
	if (!PS)
	{
		return;
	}
	PlayerHUD = CreateWidget<UPlayerHUD>(this, HUDClass);
	if (PlayerHUD)
	{
		PlayerHUD->AddToViewport();
		// Set Attributes
		PlayerHUD->SetCurrentHealth(PS->GetHealth());
		PlayerHUD->SetMaxHealth(PS->GetMaxHealth());
		PlayerHUD->SetCurrentMana(PS->GetMana());
		PlayerHUD->SetMaxMana(PS->GetMaxMana());
		PlayerHUD->SetCurrentStamina(PS->GetStamina());
		PlayerHUD->SetMaxStamina(PS->GetMaxStamina());
		ShowHUD(true);	
	}
}