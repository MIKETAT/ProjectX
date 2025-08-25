// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "XGameplayTags.h"
#include "Component/XCharacterMovementComponent.h"
#include "Logging/LogMacros.h"
#include "CharacterBase.generated.h"

class UXAttributeSet;
class AItemBase;
class UXAnimInstance;
class UXCharacterMovementComponent;
class UClimbComponent;
class UClimbSettings;
struct FOnAttributeChangeData;
class UGameplayAbility;
class UGameplayEffect;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UXGameplayAbility;
class UXAbilitySystemComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 * Character Base 类， 应该是 Player 和 Enemy 的共同基类
 */
UCLASS(config=Game)
class ACharacterBase : public ACharacter
{
	GENERATED_BODY()
public:
	// Getter Function
	// consider: 是否有多种配置的需要
	UFUNCTION()
	UClimbSettings* SelectClimbSetting() const { return ClimbSetting; }
	void ResetClimbMoveInput();
	virtual void PostInitializeComponents() override;
	void AddItem(AItemBase* Item);
	void SetOverlapItem(AItemBase* Item) { OverlapItem = Item; }
	
	virtual void PossessedBy(AController* NewController) override;
	void BindASCInput();
	virtual void InitializePassiveEffects();
	virtual void InitializePassiveAbilities();
	void InitAbilityActorInfo();

	// Status Function
	bool GetIsHanging() const { return CharMov && CharMov->bWantsToHanging; }
	
	// State Function
	UFUNCTION()
	void OnRunning(const FGameplayTag Tag, int32 NewCount);

	UFUNCTION()
	void OnSprinting(const FGameplayTag Tag, int32 NewCount);
	
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerState() override;
public:
	ACharacterBase(const FObjectInitializer& ObjInit);
	//
	virtual void Jump() override;
	virtual void StopJumping() override; 
	FCollisionQueryParams GetIgnoreCharacterParams() const;
protected:
	/** Inputs */
	void SelectMappingContext();
	void Input_OnJump();
	void Input_OnStopJumping();
	void Input_OnMove(const FInputActionValue& Value);
	void Input_OnLook(const FInputActionValue& Value);
	void Input_OnToggleInventory();
	void Input_OnInteract();
	void Input_OnClimbUp(const FInputActionValue& Value);
	void Input_OnClimbMove(const FInputActionValue& Value);
	void Input_OnClimbMove_Complete(const FInputActionValue& Value);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay() override;


	UFUNCTION()
	void OnHangStateChanged(bool NewState);
	
	UFUNCTION()
	void OnClimbUpMontageEnded(UAnimMontage* ClimbUpMontage, bool IsComplete);
	
// Variables
public:
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UXCharacterMovementComponent> CharMov;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= GAS, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UClimbComponent> ClimbComp;
	
	TWeakObjectPtr<UXAbilitySystemComponent> AbilitySystemComponent;
	TWeakObjectPtr<UXAttributeSet> AttributeSet;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GAS)
	TArray<TSubclassOf<UXGameplayAbility>> PassiveGameplayAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GAS)
	TArray<TSubclassOf<UGameplayEffect>> PassiveGameplayEffects;

	bool bPressXJump{false};
	
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AItemBase> OverlapItem;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	UArrowComponent* DebugArrow;

	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
	/** Input MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* HangingMappingContext;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* WalkAction;

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;*/
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleInventoryAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ClimbAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ClimbMoveAction;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	UXAnimInstance* AnimInstance;
		
	// Settings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	UClimbSettings* ClimbSetting;

private:
	
};

