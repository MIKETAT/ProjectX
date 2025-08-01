// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Items/InventoryHUDInterface.h"
#include "XPlayerController.generated.h"

class AItemBase;
class UInventoryWidget;
class UPlayerHUD;
class UInventoryComponent;
/**
 * 
 */
UCLASS(Blueprintable)
class PROJECTX_API AXPlayerController : public APlayerController, public IInventoryHUDInterface
{
	GENERATED_BODY()
public:
	AXPlayerController(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ToggleInventory();
	
	UFUNCTION()
	void ShowHUD(bool Visible);
	
	/*UFUNCTION(BlueprintCallable)
	void CreateHUD();*/

	// IInventoryHUDInterface
	virtual void UI_MoveInventoryItem_Implementation(int32 SourceIndex, int32 TargetIndex) override;
	// ~ End of IInventoryHUDInterface

	// 根据是否有可见的Widget切换输入模式
	void SetInputModeByWidgetVisibility();

	UFUNCTION(Server, Reliable)
	void PickupItem(AItemBase* Item);

	// TEST
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "test")
	void RemoveItemTest(int32 Index);
protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	
	void ConstructInventoryWidget();
	void ConstructGridWidget();

// Variables
	UPROPERTY(BlueprintReadWrite, Category= HUD)
	TObjectPtr<UPlayerHUD> PlayerHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	TSubclassOf<UPlayerHUD> HUDClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= "Item", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInventoryComponent> InventoryComp;

	UPROPERTY()
	TObjectPtr<UInventoryWidget> InventoryWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<UInventoryWidget> InventoryWidgetClass;

};
