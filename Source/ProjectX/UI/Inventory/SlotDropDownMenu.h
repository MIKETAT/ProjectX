// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SlotDropDownMenu.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class PROJECTX_API USlotDropDownMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UButton> Btn_Use;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UButton> Btn_Drop;

	//  Btn_Equip?
protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void BtnUseClicked();

	UFUNCTION()
	void BtnDropClicked();
};
