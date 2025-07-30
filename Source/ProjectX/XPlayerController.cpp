#include "XPlayerController.h"
#include "UI/PlayerHUD.h"
#include "Blueprint/UserWidget.h"
#include "Component/InventoryComponent.h"
#include "Items/ItemBase.h"
#include "UI/Inventory/InventoryGrid.h"
#include "UI/Inventory/InventoryWidget.h"

void AXPlayerController::RemoveItemTest_Implementation(int32 Index)
{
	ensure(InventoryComp);
	InventoryComp->RemoveItemAtIndex(Index);
}

void AXPlayerController::AcknowledgePossession(APawn* P)
{
	Super::AcknowledgePossession(P);
	// 取得网络模式和角色
	const ENetMode ANetMode = GetNetMode();
	const ENetRole ALocalRole = P->GetLocalRole();
	const ENetRole ARemoteRole = P->GetRemoteRole();

	// 只打印真正客户端那条
	if (ANetMode == NM_Client && ALocalRole == ROLE_AutonomousProxy)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[AcknowledgePossession][CLIENT] Pawn=%s Role=%s RemoteRole=%s"),
			*GetNameSafe(P),
			*UEnum::GetValueAsString(TEXT("Engine.ENetRole"), ALocalRole),
			*UEnum::GetValueAsString(TEXT("Engine.ENetRole"), ARemoteRole));
	}
	// 只打印服务器那条
	else if (HasAuthority() && ALocalRole == ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[AcknowledgePossession][SERVER] Pawn=%s Role=%s RemoteRole=%s"),
			*GetNameSafe(P),
			*UEnum::GetValueAsString(TEXT("Engine.ENetRole"), ALocalRole),
			*UEnum::GetValueAsString(TEXT("Engine.ENetRole"), ARemoteRole));
	}
}

void AXPlayerController::OnPossess(APawn* InPawn)
{

	Super::OnPossess(InPawn);
}

void AXPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ConstructInventoryWidget();
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("Server PC BeginPlay: HasPlayer=%d, PC Name=%s"), Player != nullptr, *GetName());	
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Client PC BeginPlay: HasPlayer=%d, PC Name=%s"), Player != nullptr, *GetName());
	}
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
	if (!IsLocalController() || HasAuthority() || !IsValid(InventoryWidgetClass)) {
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("ConstructInventoryWidget Before CreateWidget: Local=%d, HasPlayer=%d, Controller=%s"),
   IsLocalController(), Player != nullptr, *GetName());
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
