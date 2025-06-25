// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "XGameplayTags.h"
#include "Animation/AnimInstance.h"
#include "XGameplayTags.h"
#include "XAnimInstance.generated.h"

class ACharacterBase;
/**
 * 
 */
UCLASS()
class PROJECTX_API UXAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

// function
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
private:
	
protected:

	
// variable
public:
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	TObjectPtr<ACharacterBase> Character;
	
	// Location
	UPROPERTY(BlueprintReadWrite, Category = "Location")
	FVector WorldLocation;

	UPROPERTY(BlueprintReadWrite, Category = "Location")
	FVector WorldLocation2D;

	UPROPERTY(BlueprintReadWrite, Category = "Location")
	float DisplacementSinceLastUpdate = 0.f;		// 距离上次更新的位移大小

	UPROPERTY(BlueprintReadWrite, Category = "Location")
	float DisplacementSpeed = 0.f;				// 位移速率

	// Rotation
	UPROPERTY(BlueprintReadWrite, Category = "Rotation")
	FRotator WorldRotation;

	// Velocity
	UPROPERTY(BlueprintReadWrite, Category = "Velocity")
	FVector WorldVelocity;

	UPROPERTY(BlueprintReadWrite, Category = "Velocity")
	FVector WorldVelocity2D;

	UPROPERTY(BlueprintReadWrite, Category = "Velocity")
	FVector LocalVelocity;

	UPROPERTY(BlueprintReadWrite, Category = "Velocity")
	FVector LocalVelocity2D;

	UPROPERTY(BlueprintReadWrite, Category = "Velocity")
	float LocalVelocityDirectionAngleNoOffset;	// 速度与角色旋转前向向量 的夹角

	UPROPERTY(BlueprintReadWrite, Category = "Velocity")
	float LocalVelocityDirectionAngleWithOffset;
	
	UPROPERTY(BlueprintReadWrite, Category = "Velocity")
	bool HasVelocity;

	// Acceleration
	UPROPERTY(BlueprintReadWrite, Category = "Acceleration")
	bool HasAcceleration;
	
	UPROPERTY(BlueprintReadWrite, Category = "Acceleration")
	FVector WorldAcceleration;

	UPROPERTY(BlueprintReadWrite, Category = "Acceleration")
	FVector WorldAcceleration2D;

	UPROPERTY(BlueprintReadWrite, Category = "Acceleration")
	FVector LocalAcceleration;

	UPROPERTY(BlueprintReadWrite, Category = "Acceleration")
	FVector LocalAcceleration2D;
	
	UPROPERTY(BlueprintReadWrite)
	bool IsFirstUpdate = true;

	// State
	UPROPERTY(BlueprintReadWrite, Category = "State")
	FGameplayTag Gait{XGaitTags::Running};
};
