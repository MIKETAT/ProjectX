#include "Component/ClimbComponent.h"

#include "XCharacterMovementComponent.h"
#include "Character/CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Settings/ClimbSettings.h"

UClimbComponent::UClimbComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UClimbComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacterBase>(GetOwner());
	}
	ensure(OwnerCharacter);
}

void UClimbComponent::InitializeComponent()
{
	Super::InitializeComponent();
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacterBase>(GetOwner());
	}
	ensure(OwnerCharacter);
}

bool UClimbComponent::HangTracer()
{
	return false;
}

void UClimbComponent::GrabLedge()
{
	ensure(OwnerCharacter);
	ensure(OwnerCharacter->GetCharacterMovement());
	ensure(OwnerCharacter->GetCapsuleComponent());
	
	const float CapsuleRadius{OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius()};
	const float CapsuleHalfHeight{OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()};
	// todo: 手部最好使用IK以贴合表面
	const FVector LedgeHorizontalLocation{
		DownwardImpactPoint.X, DownwardImpactPoint.Y, 0
	};
	const FVector CharacterHorizontalLocation = LedgeHorizontalLocation + ForwardTraceNormal * CapsuleRadius;
	const float CharacterZValue = DownwardImpactPoint.Z - CapsuleHalfHeight;
	const FVector TargetLocation{CharacterHorizontalLocation.X, CharacterHorizontalLocation.Y, CharacterZValue};
	
	const FRotator TargetRotator = UKismetMathLibrary::MakeRotFromXZ(ForwardTraceNormal, FVector::UpVector);
	FLatentActionInfo LatentActionInfo;
	LatentActionInfo.CallbackTarget = this;
	LatentActionInfo.ExecutionFunction = FName("OnMoveComponentComplete");
	LatentActionInfo.UUID = FMath::Rand();
	LatentActionInfo.Linkage = 0;
	DrawDebugPoint(GetWorld(), TargetLocation, 10.f, FColor::Red, false, 20.f);
	OwnerCharacter->GetCharacterMovement()->StopMovementImmediately();
	UKismetSystemLibrary::MoveComponentTo(OwnerCharacter->GetRootComponent(), TargetLocation, TargetRotator,
		true, true, 0.1f, false, EMoveComponentAction::Move, LatentActionInfo);
}

bool UClimbComponent::CanClimbMove(EHangMoveDirection MoveDirection)
{
	const FClimbTraceSetting TraceSetting{OwnerCharacter->SelectClimbSetting()->TraceSetting};
	const ECollisionChannel ClimbTraceChannel{TraceSetting.ClimbTraceChannel};
	const float TraceCapsuleRadius{TraceSetting.TraceCapsuleRadius};
	const float HangMoveTraceLength = TraceCapsuleRadius;
	FVector HangMoveOffset{HangMoveTraceLength * WallRightDirection};
	if (MoveDirection == EHangMoveDirection::Left)
	{
		HangMoveOffset *= -1.f;
	}
	FVector StartLocation{OwnerCharacter->GetActorLocation() + HangMoveOffset};
	FVector EndLocation{StartLocation + ForwardTraceNormal * TraceSetting.ReachDistance};
	FCollisionQueryParams Params{NAME_None, false, GetOwner()};
	FHitResult Hit;
	return GetWorld()->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ClimbTraceChannel, Params);
}

void UClimbComponent::OnMoveComponentComplete()
{
	ensure(OwnerCharacter);
	ensure(OwnerCharacter->GetCharacterMovement());
	ensure(OwnerCharacter->GetController());
	
	OwnerCharacter->GetCharacterMovement()->StopMovementImmediately();
	OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Custom, static_cast<uint8>(ECustomMoveMode::CMove_Hanging));
	SetHangState(true);
	const FRotator TargetRotator = UKismetMathLibrary::MakeRotFromXZ(ForwardTraceNormal, FVector::UpVector);
	OwnerCharacter->SetActorRotation(TargetRotator);
	OwnerCharacter->GetController()->SetControlRotation(TargetRotator);
}
