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
#include "XPlayerState.h"
#include "AbilitySystem/XAbilitySystemComponent.h"
#include "AbilitySystem/XAttributeSet.h"
#include "AbilitySystem/XGameplayAbility.h"
#include "Animation/XAnimInstance.h"
#include "Component/ClimbComponent.h"
#include "Component/XCharacterMovementComponent.h"
#include "Components/ArrowComponent.h"
#include "Net/UnrealNetwork.h"
#include "Settings/ClimbSettings.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ACharacterBase::ACharacterBase(const FObjectInitializer& ObjInit)
 : Super(ObjInit.SetDefaultSubobjectClass<UXCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	SetReplicates(true);
	GetCapsuleComponent()->InitCapsuleSize(42.f, 92.0f);
	SetRootComponent(GetCapsuleComponent());
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	CharMov = Cast<UXCharacterMovementComponent>(GetCharacterMovement());
	ensure(CharMov);
	
	CharMov->bOrientRotationToMovement = false; // Character moves in the direction of input...	
	CharMov->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	CharMov->JumpZVelocity = 700.f;
	CharMov->AirControl = 0.35f;
	CharMov->MaxWalkSpeed = 200.f;
	CharMov->MinAnalogWalkSpeed = 20.f;
	CharMov->BrakingDecelerationWalking = 2000.f;
	CharMov->BrakingDecelerationFalling = 1500.0f;
	CharMov->bAlwaysCheckFloor = true;
	CharMov->PrimaryComponentTick.bCanEverTick = true;
	CharMov->SetIsReplicated(true);
	CharMov->bUseControllerDesiredRotation = true;
	CharMov->bAllowPhysicsRotationDuringAnimRootMotion = false;	// this just affects anim root motion.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Debug Arrow
	DebugArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("DebugArrow"));
	DebugArrow->SetupAttachment(RootComponent);
	DebugArrow->SetVisibility(true);
	DebugArrow->SetHiddenInGame(false);
}

FCollisionQueryParams ACharacterBase::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams Params;
	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);
	return Params;
}

void ACharacterBase::Jump()
{
	bPressXJump = true;
	Super::Jump();
	// override bPressedJump to perform Mantle and Climb
	bPressedJump = false;
}

void ACharacterBase::StopJumping()
{
	bPressXJump = false;
	Super::StopJumping();
}

void ACharacterBase::BeginPlay()
{	
	Super::BeginPlay();
	SetReplicateMovement(true);
	
	AnimInstance = Cast<UXAnimInstance>(GetMesh()->GetAnimInstance());
	if (!CharMov)
	{
		CharMov = Cast<UXCharacterMovementComponent>(GetCharacterMovement());
	}
	if (CharMov)
	{
		CharMov->OnHangStateChanged.AddDynamic(this, &ThisClass::OnHangStateChanged);
	}
}

void ACharacterBase::OnRunning(const FGameplayTag Tag, int32 NewCount)
{
	CharMov = Cast<UXCharacterMovementComponent>(GetCharacterMovement());
	ensure(CharMov);
	if (!Tag.MatchesTag(XGaitTags::Running))
	{
		return;
	}
	if (NewCount > 0)
	{
		CharMov->SetRunning(true);	
	} else
	{
		CharMov->SetRunning(false);
	}
}

void ACharacterBase::OnSprinting(const FGameplayTag Tag, int32 NewCount)
{
	CharMov = Cast<UXCharacterMovementComponent>(GetCharacterMovement());
	ensure(CharMov);
	if (!Tag.MatchesTag(XGaitTags::Sprinting))
	{
		return;
	}
	if (NewCount > 0)
	{
		CharMov->SetSprinting(true);
	} else
	{
		CharMov->SetSprinting(false);
	}
}

void ACharacterBase::SelectMappingContext()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (GetIsHanging())
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

void ACharacterBase::OnHangStateChanged(bool NewState)
{
	SelectMappingContext();
	UE_LOG(ProjectX, Error, TEXT("OnHangStateChanged BoardCast, GetIsHanging() = %d"), GetIsHanging());
}

void ACharacterBase::OnClimbUpMontageEnded(UAnimMontage* ClimbUpMontage, bool Interrupted)
{
	ensure(CharMov);
	// 如果 Interrupted 也认为离开了Hanging状态
	if (ClimbUpMontage)
	{
		CharMov->SetMovementMode(MOVE_Walking);
		CharMov->SetWantsToHanging(false);
	} else
	{
		CharMov->SetMovementMode(MOVE_Custom, static_cast<uint8>(ECustomMoveMode::CMove_Hanging));
		CharMov->SetWantsToHanging(true);
	}
}

void ACharacterBase::ResetClimbMoveInput()
{
	ensure(AnimInstance);
	if (CharMov)
	{
		CharMov->SetHangInput(0);
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
}

void ACharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilityActorInfo();
	
	if (AXPlayerState* PS = GetPlayerState<AXPlayerState>())
	{
		AttributeSet = PS->GetAttributeSet();
	}
	if (AXPlayerController* PC = Cast<AXPlayerController>(GetController()))
	{
		PC->CreateHUD();
	}
}

// Server only
void ACharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilityActorInfo();
	
	// 对于没有PlayerController的AI敌人，这里是初始化Owner 和 Avatar 的时机
	if (AXPlayerState* PS = GetPlayerState<AXPlayerState>())
	{
		AttributeSet = PS->GetAttributeSet();
		InitializePassiveAbilities();
		InitializePassiveEffects();
		
		if (AXPlayerController* PC = Cast<AXPlayerController>(NewController))
		{
			PC->CreateHUD();
			SetOwner(NewController);
		}	
	}
}

void ACharacterBase::BindASCInput()
{
	// GAS
	if (AbilitySystemComponent.IsValid() && IsValid(InputComponent))
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
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent,Binds);
	}
}

void ACharacterBase::InitializePassiveEffects()
{
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent.IsValid())
	{
		return;
	}
	for (TSubclassOf<UGameplayEffect>& Effect : PassiveGameplayEffects)
	{
		FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
		ContextHandle.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Effect, 1, ContextHandle);
		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle EffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), AbilitySystemComponent.Get());	
		}
	}
}

void ACharacterBase::InitializePassiveAbilities()
{
	// Grant Abilities only on Server.  todo: need a bInitialized?
	if (GetLocalRole() != ROLE_Authority || !AbilitySystemComponent.IsValid())
	{
		return;
	}
	for (TSubclassOf<UXGameplayAbility>& Ability : PassiveGameplayAbilities)
	{
		// Ability(nullptr), Level(1), InputID(INDEX_NONE), SourceObject(nullptr)
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 1, static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), this));
	}
}

void ACharacterBase::InitAbilityActorInfo()
{
	if (AXPlayerState* PS = GetPlayerState<AXPlayerState>())
	{
		if (UXAbilitySystemComponent* ASC = Cast<UXAbilitySystemComponent>(PS->GetAbilitySystemComponent()))
		{
			// If not Initialized
			if (!AbilitySystemComponent.IsValid())
			{
				AbilitySystemComponent = ASC;
			}
			AbilitySystemComponent->InitAbilityActorInfo(PS, this);
			// Bind Delegate
			BindASCInput();

			if (!CharMov)
			{
				CharMov = Cast<UXCharacterMovementComponent>(GetCharacterMovement());
			}
			ensure(CharMov);
			AbilitySystemComponent->RegisterGameplayTagEvent(XGaitTags::Running, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ACharacterBase::OnRunning);
			AbilitySystemComponent->RegisterGameplayTagEvent(XGaitTags::Sprinting, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ACharacterBase::OnSprinting);
		
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
	BindASCInput();
}

void ACharacterBase::Input_OnJump()
{
	Jump();
}

void ACharacterBase::Input_OnStopJumping()
{
	//Super::StopJumping();
	StopJumping();
}

void ACharacterBase::Input_OnMove(const FInputActionValue& Value)
{
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

void ACharacterBase::Input_OnLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ACharacterBase::Input_OnClimbUp(const FInputActionValue& Value)
{
	ensure(CharMov);
	if (AnimInstance && ClimbSetting->ClimbMontage)
	{
		CharMov->SetMovementMode(MOVE_Flying);
		AnimInstance->Montage_Play(ClimbSetting->ClimbMontage, 1.f);
		FOnMontageEnded ClimbUpCompleteDelegate;
		ClimbUpCompleteDelegate.BindUObject(this, &ThisClass::OnClimbUpMontageEnded);
		AnimInstance->Montage_SetEndDelegate(ClimbUpCompleteDelegate, ClimbSetting->ClimbMontage);
	}
}

void ACharacterBase::Input_OnClimbMove(const FInputActionValue& Value)
{
	ensure(AnimInstance);
	ensure(CharMov);
	if (Controller != nullptr)
	{
		FVector2D MovementVector = Value.Get<FVector2d>();
		EHangMoveDirection Direction = MovementVector.X > 0 ? EHangMoveDirection::Right : EHangMoveDirection::Left;
		AnimInstance->SetHangMoveDirection(Direction);
		CharMov->SetHangInput(MovementVector.X);
	}
}

void ACharacterBase::Input_OnClimbMove_Complete(const FInputActionValue& Value)
{
	ensure(AnimInstance);
	AnimInstance->SetHangMoveDirection(EHangMoveDirection::None);
	if (CharMov)
	{
		CharMov->SetHangInput(0);	
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
