// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "XGameplayTags.h"
#include "Logging/LogMacros.h"
#include "CharacterBase.generated.h"

struct FOnAttributeChangeData;
class UGameplayAbility;
class UGameplayEffect;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

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
	inline const FGameplayTag& GetGait() const { return Gait; }

protected:	
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	// GAS
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= GAS, meta = (AllowPrivateAccess = "true"))
	class UXAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category= GAS, meta = (AllowPrivateAccess = "true"))
	class UXAttributeSet* AttributeSet;
	
	// Input
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* WalkAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* CrouchAction;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", Transient)
	FGameplayTag Gait{XGaitTags::Running};

private:
	
public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void InitializePassiveEffects();
	virtual void InitializePassiveAbilities();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GAS)
	TArray<TSubclassOf<UGameplayEffect>> PassiveGameplayEffects;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = GAS)
	TArray<TSubclassOf<UGameplayAbility>> PassiveGameplayAbilities;

	ACharacterBase();

	// Attribute Getter
	UFUNCTION(BlueprintCallable)
	virtual float GetHealth() const;		// idk why in actionRPG it's virtual

	UFUNCTION(BlueprintCallable)
	virtual float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable)
	virtual float GetStamina() const;

	UFUNCTION(BlueprintCallable)
	virtual float GetMaxStamina() const;
	
protected:
	virtual void Tick(float DeltaSeconds) override;
	
	/** Inputs */
	void Input_OnMove(const FInputActionValue& Value);
	void Input_OnLook(const FInputActionValue& Value);
	void Input_OnWalk();
	void Input_OnSprint(const FInputActionValue& Value);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

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
	
	/*UFUNCTION(BlueprintImplementableEvent)
	void OnMaxHealthChanged(float Value, const struct FGameplayTagContainer& EventTag);

	UFUNCTION(BlueprintImplementableEvent)
	void OnStaminaChanged(float Value, const struct FGameplayTagContainer& EventTag);

	UFUNCTION(BlueprintImplementableEvent)
	void OnMaxStaminaChanged(float Value, const struct FGameplayTagContainer& EventTag);*/
	

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};

