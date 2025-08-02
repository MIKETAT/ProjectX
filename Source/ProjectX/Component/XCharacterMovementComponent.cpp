#include "Component/XCharacterMovementComponent.h"
#include "GameFramework/Character.h"

void UXCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if (MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECustomMoveMode::CMove_Hanging))
	{
		GravityScale = 0.f;
		Velocity = FVector::ZeroVector;
	} else if (MovementMode == MOVE_Walking)
	{
		GravityScale = 1.f;
	}
}

void UXCharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	if (CustomMovementMode == static_cast<uint8>(ECustomMoveMode::CMove_Hanging))
	{
		PhysHanging(deltaTime, Iterations);
	}
	Super::PhysCustom(deltaTime, Iterations);
}

FVector UXCharacterMovementComponent::ConstrainAnimRootMotionVelocity(const FVector& RootMotionVelocity,
	const FVector& CurrentVelocity) const
{
	if (MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECustomMoveMode::CMove_Hanging))
	{
		return Velocity;
	}
	return Super::ConstrainAnimRootMotionVelocity(RootMotionVelocity, CurrentVelocity);
}

void UXCharacterMovementComponent::PhysHanging(float deltaTime, int32 Iterations)
{
	if (!CharacterOwner || !UpdatedComponent)
	{
		return;
	}
	FVector RightDirection = FVector::CrossProduct(FVector::UpVector, -WallNormal).GetSafeNormal();
	const FVector HangVelocity = RightDirection * HangMoveInput * HangMoveSpeed;
	FVector MoveDelta = HangVelocity * deltaTime;
	FHitResult HitResult;
	SafeMoveUpdatedComponent(MoveDelta, CharacterOwner->GetActorRotation(), true, HitResult);
}

void FSavedMove_HangingMovement::Clear()
{
	Super::Clear();
	bSavedWantsToHanging = 0;
}

uint8 FSavedMove_HangingMovement::GetCompressedFlags() const
{
	uint8 Result =  FSavedMove_Character::GetCompressedFlags(); // jump and crouch
	if (bSavedWantsToHanging)
	{
		Result |= FLAG_Custom_0;
	}
	return Result;
}

// 返回能否合并(NewMove与当前是否一致)
bool FSavedMove_HangingMovement::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter,
	float MaxDelta) const
{
	if (bSavedWantsToHanging != ((FSavedMove_HangingMovement*)&NewMove)->bSavedWantsToHanging)
	{
		return false;
	}
	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_HangingMovement::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);
	UXCharacterMovementComponent* CharMov = Cast<UXCharacterMovementComponent>(C->GetCharacterMovement());
	if (CharMov)
	{
		bSavedWantsToHanging = CharMov->bWantsToHanging;
	}
	Acceleration.X = FMath::RoundToFloat(Acceleration.X);
	Acceleration.Y = FMath::RoundToFloat(Acceleration.Y);
	Acceleration.Z = FMath::RoundToFloat(Acceleration.Z);
}

void FSavedMove_HangingMovement::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);
	UXCharacterMovementComponent* CharMov = Cast<UXCharacterMovementComponent>(C->GetCharacterMovement());
	if (CharMov)
	{
		CharMov->bWantsToHanging = bSavedWantsToHanging;
	}
}

FNetworkPredictionData_Client_Character_HangingMovement::FNetworkPredictionData_Client_Character_HangingMovement(
	const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_Character_HangingMovement::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_HangingMovement());
}

UXCharacterMovementComponent::UXCharacterMovementComponent(const FObjectInitializer& ObjInit)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	SetIsReplicated(true);
}

void UXCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bWantsToHanging = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

// todo
FNetworkPredictionData_Client* UXCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);
	if (!ClientPredictionData)
	{
		UXCharacterMovementComponent* MutableThis = const_cast<UXCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Character_HangingMovement(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}
	return ClientPredictionData;
}

void UXCharacterMovementComponent::SetWallNormal(FVector NewWallNormal)
{
	WallNormal = NewWallNormal;
}

void UXCharacterMovementComponent::SetHangInput(float Input)
{
	HangMoveInput = Input;
}

void UXCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	SetMovementMode(MOVE_Walking);
}
