#include "XPlayerController.h"
#include "UI/PlayerHUD.h"
#include "Blueprint/UserWidget.h"
#include "Component/InventoryComponent.h"
#include "Items/ItemBase.h"
#include "UI/Inventory/InventoryWidget.h"

void AXPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
}

void AXPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ConstructInventoryWidget();
}

void AXPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

AXPlayerController::AXPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {
	InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	bReplicates = true;
	//CreateHUD();
}

void AXPlayerController::ToggleInventory()
{
	if (!IsValid(InventoryWidget))
	{
		UE_LOG(LogTemp, Error, TEXT("InventoryWidget is not valid!"));
		return;
	}
	InventoryWidget->ToggleWindow();
	SetInputModeByWidgetVisibility();
	// todo: broadcast event
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
	// todo: check hud valid
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

UE_DISABLE_OPTIMIZATION
void AXPlayerController::PickupItem_Implementation(AItemBase* Item)
{
	if (InventoryComp && Item)
	{
		InventoryComp->AddItemDefinition(Item->GetItemDefinition());
	}
}
UE_ENABLE_OPTIMIZATION
// ~ End of IInventoryHUDInterface


void AXPlayerController::ConstructInventoryWidget()
{
	if (!IsLocalController() || !IsValid(InventoryWidgetClass)) {
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


/*void AXPlayerController::CreateHUD()
{
	if (PlayerHUD)
	{
		ShowHUD(true);
	} else
	{
		ensure(HUDClass);
		PlayerHUD = CreateWidget<UPlayerHUD>(this, HUDClass);
		if (PlayerHUD)
		{
			PlayerHUD->AddToViewport();
			ShowHUD(true);
		}
	}
}*/
