// Fill out your copyright notice in the Description page of Project Settings.


#include "SlotDropDownMenu.h"

#include "Components/Button.h"

void USlotDropDownMenu::NativeConstruct()
{
	Super::NativeConstruct();

	Btn_Use->OnClicked.AddUniqueDynamic(this, &ThisClass::BtnUseClicked);
	Btn_Drop->OnClicked.AddUniqueDynamic(this, &ThisClass::BtnDropClicked);
}

void USlotDropDownMenu::BtnUseClicked()
{
	UE_LOG(LogTemp, Error, TEXT("BtnUseClicked"));
}

void USlotDropDownMenu::BtnDropClicked()
{
	UE_LOG(LogTemp, Error, TEXT("BtnDropClicked"));
}
