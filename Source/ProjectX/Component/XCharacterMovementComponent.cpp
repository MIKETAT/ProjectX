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
		return;
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

void UXCharacterMovementComponent::SetWallNormal(FVector NewWallNormal)
{
	WallNormal = NewWallNormal;
}

void UXCharacterMovementComponent::SetHangInput(float Input)
{
	HangMoveInput = Input;
}
