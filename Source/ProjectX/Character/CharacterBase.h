// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "XGameplayTags.h"
#include "Logging/LogMacros.h"
#include "CharacterBase.generated.h"

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
 * 
 */
UCLASS(config=Game)
class ACharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	FORCEINLINE const FGameplayTag& GetGait() const { return Gait; }

	// Getter Function
	// consider: 是否有多种配置的需要
	UFUNCTION()
	UClimbSettings* SelectClimbSetting() const { return ClimbSetting; }
	bool GetIsHanging() const { return bIsHanging; }
	void ResetClimbMoveInput();
	virtual void PostInitializeComponents() override;

	void AddItem(AItemBase* Item);
	void SetOverlapItem(AItemBase* Item) { OverlapItem = Item; }
protected:
	// test
	UPROPERTY()
	TObjectPtr<AItemBase> OverlapItem;
	// ~ end of test
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	UArrowComponent* DebugArrow;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// GAS
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= GAS, meta = (AllowPrivateAccess = "true"))
	UXAbilitySystemComponent* ASC;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= GAS, meta = (AllowPrivateAccess = "true"))
	class UXAttributeSet* AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= GAS, meta = (AllowPrivateAccess = "true"))
	UClimbComponent* ClimbComp;
	
	// Input
	/** MappingContext */
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;
	
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
	
	// States
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag Gait{XGaitTags::Running};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	bool bIsHanging{false};
	
	// Settings;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	UClimbSettings* ClimbSetting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
	UXAnimInstance* AnimInstance;
private:
	
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void InitializePassiveEffects();
	virtual void InitializePassiveAbilities();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GAS)
	TArray<TSubclassOf<UXGameplayAbility>> PassiveGameplayAbilities;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GAS)
	TArray<TSubclassOf<UGameplayEffect>> PassiveGameplayEffects;

	ACharacterBase(const FObjectInitializer& ObjInit);

	// Attribute Getter
	UFUNCTION(BlueprintCallable)
	virtual float GetHealth() const;		// idk why in actionRPG it's virtual

	UFUNCTION(BlueprintCallable)
	virtual float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable)
	virtual float GetMana() const;

	UFUNCTION(BlueprintCallable)
	virtual float GetMaxMana() const;
	
	UFUNCTION(BlueprintCallable)
	virtual float GetStamina() const;

	UFUNCTION(BlueprintCallable)
	virtual float GetMaxStamina() const;
	
protected:
	virtual void Tick(float DeltaSeconds) override;
	
	/** Inputs */
	void SelectMappingContext();
	void Input_OnJump();
	void Input_OnStopJumping();
	void Input_OnMove(const FInputActionValue& Value);
	void Input_OnLook(const FInputActionValue& Value);
	void Input_OnWalk();
	void Input_OnSprint(const FInputActionValue& Value);
	void Input_OnClimbUp(const FInputActionValue& Value);
	void Input_OnClimbMove(const FInputActionValue& Value);
	void Input_OnClimbMove_Complete(const FInputActionValue& Value);
	void Input_OnToggleInventory();
	void Input_OnInteract();
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay() override;

	/**
	 * Property Callback
	 */
	/**
	 * @param Value :		Delta Value
	 * @param EventTag:		GameplayEvent Tag that change Health/MaxHealth/Stamina/MaxStamina
	 */
	//UFUNCTION(BlueprintImplementableEvent)
	//void OnHealthChanged(float Value, const struct FGameplayTagContainer& EventTag);
	void OnHealthChanged(const FOnAttributeChangeData& Data);

	// Attribute Change Event in Blueprint
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChangeEvent, float, NewHealth);

	UPROPERTY(BlueprintAssignable, Category = GAS)
	FOnHealthChangeEvent HealthChangeEvent;
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnStaminaChanged(float Value, const struct FGameplayTagContainer& EventTag);

	UFUNCTION(BlueprintImplementableEvent)
	void OnMaxStaminaChanged(float Value, const struct FGameplayTagContainer& EventTag);

	UFUNCTION()
	void OnHangStateChanged(bool NewState);
	
	UFUNCTION()
	void OnClimbUpMontageEnded(UAnimMontage* ClimbUpMontage, bool IsComplete);
	

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

