// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "XPlayerController.generated.h"

class UPlayerHUD;
/**
 * 
 */
UCLASS()
class PROJECTX_API AXPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, Category= HUD)
	UPlayerHUD* PlayerHUD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = HUD)
	TSubclassOf<UPlayerHUD> HUDClass;
	
	UFUNCTION()
	void ShowHUD(bool Visible);
	
	UFUNCTION(BlueprintCallable)
	void CreateHUD();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void BeginPlay() override;
};
