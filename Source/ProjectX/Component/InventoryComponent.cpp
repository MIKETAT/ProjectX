#include "Component/InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInventoryComponent::InitializeComponent()
{
	UActorComponent::InitializeComponent();
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UInventoryComponent::AddItem(const FPrimaryAssetId& ItemId, int32 Count)
{
	return true;
}

bool UInventoryComponent::RemoveItem(const FItemSlot& Slot, int32 Count)
{
	return true;
}

bool UInventoryComponent::UseItem(const FItemSlot& Slot, int32 Count)
{
	return true;
}


