// Copyright Epic Games, Inc. All Rights Reserved.
#include "CharacterBase.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "XPlayerController.h"
#include "AbilitySystem/XAbilitySystemComponent.h"
#include "AbilitySystem/XAttributeSet.h"
#include "AbilitySystem/XGameplayAbility.h"
#include "Animation/XAnimInstance.h"
#include "Component/ClimbComponent.h"
#include "Component/InventoryComponent.h"
#include "Component/XCharacterMovementComponent.h"
#include "Components/ArrowComponent.h"
#include "Items/ItemBase.h"
#include "Net/UnrealNetwork.h"
#include "Settings/ClimbSettings.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ACharacterBase::ACharacterBase(const FObjectInitializer& ObjInit)
 : Super(ObjInit.SetDefaultSubobjectClass<UXCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	SetReplicates(true);
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	SetRootComponent(GetCapsuleComponent());
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->bAlwaysCheckFloor = true;

	GetCharacterMovement()->PrimaryComponentTick.bCanEverTick = true;
	GetCharacterMovement()->SetIsReplicated(true);
	
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// GAS
	ASC = CreateDefaultSubobject<UXAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	ASC->SetIsReplicated(true);
	ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	AttributeSet = CreateDefaultSubobject<UXAttributeSet>(TEXT("AttributeSet"));

	// Climb Component
	ClimbComp = CreateDefaultSubobject<UClimbComponent>(TEXT("ClimbComp"));
	
	// Debug Arrow
	DebugArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DebugArrow"));
	DebugArrow->SetupAttachment(RootComponent);
	DebugArrow->SetVisibility(true);
	DebugArrow->SetHiddenInGame(false);

	
}

void ACharacterBase::Server_SetGait_Implementation(FGameplayTag NewGait)
{
	Gait = NewGait;
	if (Gait == XGaitTags::Walking)
	{
		GetCharacterMovement()->MaxWalkSpeed = 375.f;
	} else if (Gait == XGaitTags::Running)
	{
		GetCharacterMovement()->MaxWalkSpeed = 500.f;
	} else if (Gait == XGaitTags::Sprinting)
	{
		// todo: do not hard code
		GetCharacterMovement()->MaxWalkSpeed = 700.f;
	}
}

void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	SetReplicateMovement(true);
	ensure(ASC);
	ASC->InitAbilityActorInfo(this, this);

	// Register Attribute Change Event
	ASC->GetGameplayAttributeValueChangeDelegate(UXAttributeSet::GetHealthAttribute()).AddUObject(this, &ACharacterBase::OnHealthChanged);
	
	if (ClimbComp)
	{
		ClimbComp->OnHangStateChanged.AddDynamic(this, &ThisClass::OnHangStateChanged);
	}
	AnimInstance = Cast<UXAnimInstance>(GetMesh()->GetAnimInstance());

	// debug

	const auto* MoveComp = GetCharacterMovement();
	ENetworkSmoothingMode Mode = MoveComp->NetworkSmoothingMode;
	UE_LOG(LogTemp, Warning,
		TEXT(">>> [%s] NetworkSmoothingMode = %s"),
		*GetNameSafe(this),
		*UEnum::GetValueAsString(TEXT("Engine.ENetworkSmoothingMode"), Mode)
	);
}

float ACharacterBase::GetHealth() const
{
	if (!AttributeSet)		return 0.f;
	return AttributeSet->GetHealth();
}

float ACharacterBase::GetMaxHealth() const
{
	if (!AttributeSet)		return 0.f;
	return AttributeSet->GetMaxHealth();
}

float ACharacterBase::GetMana() const
{
	if (!AttributeSet)		return 0.f;
	return AttributeSet->GetMana();
}

float ACharacterBase::GetMaxMana() const
{
	if (!AttributeSet)		return 0.f;
	return AttributeSet->GetMaxMana();
}

float ACharacterBase::GetStamina() const
{
	if (!AttributeSet)		return 0.f;
	return AttributeSet->GetStamina();
}

float ACharacterBase::GetMaxStamina() const
{
	if (!AttributeSet)		return 0.f;
	return AttributeSet->GetMaxStamina();
}

void ACharacterBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
}

void ACharacterBase::SelectMappingContext()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (bIsHanging)
			{
				Subsystem->RemoveMappingContext(DefaultMappingContext);
				Subsystem->AddMappingContext(HangingMappingContext, 0);
			} else
			{
				Subsystem->RemoveMappingContext(HangingMappingContext);
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}

void ACharacterBase::OnHealthChanged(const FOnAttributeChangeData& Data)
{
	HealthChangeEvent.Broadcast(Data.NewValue);
}

void ACharacterBase::OnHangStateChanged(bool NewState)
{
	if (bIsHanging == NewState)
	{
		return;
	}
	bIsHanging = NewState;
	if (AnimInstance)
	{
		AnimInstance->SetHangState(NewState);
	}
	SelectMappingContext();
}

void ACharacterBase::OnClimbUpMontageEnded(UAnimMontage* ClimbUpMontage, bool Interrupted)
{
	ensure(ClimbComp);
	ensure(GetCharacterMovement());
	
	if (ClimbUpMontage && !Interrupted)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
		ClimbComp->SetHangState(false);
	} else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Custom, static_cast<uint8>(ECustomMoveMode::CMove_Hanging));
		ClimbComp->SetHangState(true);
	}
}

void ACharacterBase::ResetClimbMoveInput()
{
	ensure(AnimInstance);
	if (UXCharacterMovementComponent* UxCharacterMovementComp = Cast<UXCharacterMovementComponent>(GetCharacterMovement()))
	{
		UxCharacterMovementComp->SetHangInput(0);
		AnimInstance->SetHangMoveDirection(EHangMoveDirection::None);
	}
}

void ACharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
}

void ACharacterBase::AddItem(AItemBase* Item)
{
	
}

void ACharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, Gait);
}

bool ACharacterBase::IsLocallyControlled() const
{
	if (HasAuthority() && GetLocalRole() == ROLE_AutonomousProxy)
	{
		return false;
	}
	return Super::IsLocallyControlled();
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return ASC;
}

void ACharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializePassiveAbilities();
	InitializePassiveEffects();


	ENetRole ALocalRole = GetLocalRole();
	ENetRole ARemoteRole = GetRemoteRole();
	bool bIsLocalCtrl = IsLocallyControlled();

	UE_LOG(LogTemp, Warning,
	  TEXT("[PossessedBy] Controller=%s | Role=%s | RemoteRole=%s | IsLocallyControlled=%d"),
	  *GetNameSafe(NewController),
	  *RoleToString(ALocalRole),
	  *RoleToString(ARemoteRole),
	  bIsLocalCtrl ? 1 : 0);
	
}

// initialize passive GA
void ACharacterBase::InitializePassiveAbilities()
{
	ensure(ASC);
	for (TSubclassOf<UXGameplayAbility>& Ability : PassiveGameplayAbilities)
	{
		ASC->GiveAbility(FGameplayAbilitySpec(Ability));
	}
}

// initialize passive GE
void ACharacterBase::InitializePassiveEffects()
{
	ensure(ASC);
	for (TSubclassOf<UGameplayEffect>& Effect : PassiveGameplayEffects)
	{
		FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(Effect, 1, ContextHandle);
		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle EffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());	
		}
	}
}



//////////////////////////////////////////////////////////////////////////
// Input
void ACharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	SelectMappingContext();
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::Input_OnJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::Input_OnStopJumping);
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnMove);
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnLook);
		// Walk/Sprint/Crouch
		EnhancedInputComponent->BindAction(WalkAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnWalk);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ThisClass::Input_OnSprint);
		//EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ACharacterBase::);
		
		// Hanging
		EnhancedInputComponent->BindAction(ClimbAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnClimbUp);
		EnhancedInputComponent->BindAction(ClimbMoveAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnClimbMove);
		EnhancedInputComponent->BindAction(ClimbMoveAction, ETriggerEvent::Completed, this, &ThisClass::Input_OnClimbMove_Complete);

		// Inventory
		EnhancedInputComponent->BindAction(ToggleInventoryAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnToggleInventory);

		// Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ThisClass::Input_OnInteract);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	// GAS
	if (ASC && PlayerInputComponent)
	{
		const FTopLevelAssetPath InputEnumPath(
			TEXT("/Script/ProjectX"),       
			TEXT("EAbilityInputID")  
		);
		FGameplayAbilityInputBinds Binds(
			TEXT("Confirm"),
			TEXT("Cancel"),
			InputEnumPath,
			static_cast<uint8>(EAbilityInputID::Confirm),
			static_cast<uint8>(EAbilityInputID::Cancel)
		);
		
		ASC->BindAbilityActivationToInputComponent(PlayerInputComponent,Binds);
	}
}

void ACharacterBase::Input_OnJump()
{
	Super::Jump();
	ensure(ClimbComp);
	if (ClimbComp->HangTracer())
	{
		ClimbComp->GrabLedge();
	} else
	{
		UE_LOG(LogTemp, Error, TEXT("Can't Hang"));
	}
}

void ACharacterBase::Input_OnStopJumping()
{
	Super::StopJumping();
}

void ACharacterBase::Input_OnMove(const FInputActionValue& Value)
{
	if (Controller && Controller->IsLocalController()
		&& GetLocalRole() == ROLE_AutonomousProxy)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[MoveForward][CLIENT] Value=%s"), *Value.ToString());
		FVector2D MovementVector = Value.Get<FVector2D>();
		if (Controller != nullptr)
		{
			const FRotator Rotation = Controller->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}
		
		
	}


}

void ACharacterBase::Input_OnLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACharacterBase::Input_OnWalk()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Input_OnWalk"));
	if (Gait == XGaitTags::Walking)
	{
		Gait = XGaitTags::Running;
		GetCharacterMovement()->MaxWalkSpeed = 500.f;
		// todo: 后续类似的改动通过refresh或者进行更新
	} else if (Gait == XGaitTags::Running)
	{
		Gait = XGaitTags::Walking;
		GetCharacterMovement()->MaxWalkSpeed = 375.f;
	}
	Server_SetGait(Gait);
}

void ACharacterBase::Input_OnSprint(const FInputActionValue& Value)
{
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Input_OnSprint"));
	Gait = Value.Get<bool>() ? XGaitTags::Sprinting : XGaitTags::Running;
	GetCharacterMovement()->MaxWalkSpeed = 700.f;
	Server_SetGait(Gait);
}

void ACharacterBase::Input_OnClimbUp(const FInputActionValue& Value)
{
	ensure(ClimbComp);
	ensure(GetCharacterMovement());
	
	if (AnimInstance && ClimbSetting->ClimbMontage)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		AnimInstance->Montage_Play(ClimbSetting->ClimbMontage, 1.f);
		FOnMontageEnded ClimbUpCompleteDelegate;
		ClimbUpCompleteDelegate.BindUObject(this, &ThisClass::OnClimbUpMontageEnded);
		AnimInstance->Montage_SetEndDelegate(ClimbUpCompleteDelegate, ClimbSetting->ClimbMontage);
	}
}

void ACharacterBase::Input_OnClimbMove(const FInputActionValue& Value)
{
	ensure(ClimbComp);
	ensure(AnimInstance);
	if (Controller != nullptr)
	{
		FVector2D MovementVector = Value.Get<FVector2d>();
		EHangMoveDirection Direction = MovementVector.X > 0 ? EHangMoveDirection::Right : EHangMoveDirection::Left;
		if (ClimbComp->CanClimbMove(Direction))
		{
			AnimInstance->SetHangMoveDirection(Direction);
			if (UXCharacterMovementComponent* UxCharacterMovementComp = Cast<UXCharacterMovementComponent>(GetCharacterMovement()))
			{
				UxCharacterMovementComp->SetHangInput(MovementVector.X);	
			}
		} else
		{
			ResetClimbMoveInput();
		}
	}
}

void ACharacterBase::Input_OnClimbMove_Complete(const FInputActionValue& Value)
{
	ensure(AnimInstance);
	AnimInstance->SetHangMoveDirection(EHangMoveDirection::None);
	if (UXCharacterMovementComponent* UxCharacterMovementComp = Cast<UXCharacterMovementComponent>(GetCharacterMovement()))
	{
		UxCharacterMovementComp->SetHangInput(0);	
	}
}

void ACharacterBase::Input_OnToggleInventory()
{
	if (AXPlayerController* PlayerController = Cast<AXPlayerController>(GetController()))
	{
		PlayerController->ToggleInventory();
	}
}

void ACharacterBase::Input_OnInteract()
{
	if (AXPlayerController* PlayerController = Cast<AXPlayerController>(GetController()))
	{
		PlayerController->PickupItem(OverlapItem);
	}
}
