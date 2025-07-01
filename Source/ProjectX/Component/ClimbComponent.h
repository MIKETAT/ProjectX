// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClimbComponent.generated.h"

class ACharacterBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTX_API UClimbComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UClimbComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void InitializeComponent() override;
	
	// Check if Character Can Hang on a Wall
	UFUNCTION(BlueprintCallable, Category = "Climb", Meta = (ReturnDisplayName = "CanHang"))
	bool HangTracer();
	
// Variables
private:
	UPROPERTY()
	ACharacterBase* OwnerCharacter;
};
