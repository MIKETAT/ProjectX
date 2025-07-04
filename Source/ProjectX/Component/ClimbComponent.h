// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ClimbComponent.generated.h"

class ACharacterBase;


UENUM(BlueprintType)
enum class EHangMoveDirection : uint8
{
	None UMETA(DisplyName = "None"),
	Left UMETA(DisplyName = "Left"),
	Right UMETA(DisplyName = "Right")
	// 
};


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

	UFUNCTION(BlueprintCallable, Category = "Climb")
	void GrabLedge();
	
// Variables
private:
	UPROPERTY()
	ACharacterBase* OwnerCharacter;

	UPROPERTY()
	FVector ForwardTraceNormal;

	UPROPERTY()
	FVector ForwardImpactPoint;

	UPROPERTY()
	FVector DownwardImpactPoint;

	UPROPERTY()
	FVector WallRightDirection;

	UPROPERTY()
	float HangMoveTraceDownLength{100.f};
	
public:
	UPROPERTY(BlueprintReadWrite, Category = "States")
	bool bIsHanging{false};

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHangStateChanged, bool, bNewState);
	UPROPERTY(BlueprintAssignable, Category = "Climb")
	FOnHangStateChanged OnHangStateChanged;

	bool CanClimbMove(EHangMoveDirection MoveDirection);
	
	UFUNCTION(BlueprintCallable)
	void SetHangState(bool NewState);

	// Callback
	UFUNCTION()
	void OnMoveCompoComplete();
};
