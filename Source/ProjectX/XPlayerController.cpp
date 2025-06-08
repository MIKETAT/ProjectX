// Fill out your copyright notice in the Description page of Project Settings.


#include "XPlayerController.h"
#include "ProjectX.h"
#include "UI/PlayerHUD.h"
#include "Blueprint/UserWidget.h"

void AXPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	CreateHUD();
}

void AXPlayerController::BeginPlay()
{
	Super::BeginPlay();
}


void AXPlayerController::ShowHUD(bool Visible)
{
	check(PlayerHUD);
	if (Visible)
	{
		PlayerHUD->SetVisibility(ESlateVisibility::Visible);
	} else
	{
		PlayerHUD->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AXPlayerController::CreateHUD()
{
	if (PlayerHUD)
	{
		ShowHUD(true);
	} else
	{
		check(HUDClass);
		PlayerHUD = CreateWidget<UPlayerHUD>(this, HUDClass);
		if (PlayerHUD)
		{
			PlayerHUD->AddToViewport();
			ShowHUD(true);
		}
	}
}
