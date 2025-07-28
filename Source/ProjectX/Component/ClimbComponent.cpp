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

void UClimbComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
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
	ensure(OwnerCharacter);
	ensure(OwnerCharacter->GetMesh());
	ensure(OwnerCharacter->GetCharacterMovement());
	ensure(OwnerCharacter->GetCapsuleComponent());
	// Settings Parameters
	const FClimbTraceSetting TraceSetting{OwnerCharacter->SelectClimbSetting()->TraceSetting};
	const float ReachDistance{TraceSetting.ReachDistance};
	const float TraceCapsuleRadius{TraceSetting.TraceCapsuleRadius};
	const ECollisionChannel ClimbTraceChannel{TraceSetting.ClimbTraceChannel};
	const float LedgeHeightDelta{TraceSetting.LedgeHeight.GetMax() - TraceSetting.LedgeHeight.GetMin()};
	const float ForwardTraceCapsuleHalfHeight{LedgeHeightDelta * 0.5f};
	
	// Location Size... Parameters
	const FVector ActorLocation{OwnerCharacter->GetActorLocation()};
	const float CapsuleHalfHeight{OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()};
	const FVector CapsuleBottomLocation{ActorLocation.X, ActorLocation.Y, ActorLocation.Z - CapsuleHalfHeight};
	const double CapsuleScale{OwnerCharacter->GetCapsuleComponent()->GetComponentScale().Z};
	
	// Trace Result Parameters
	FHitResult ForwardTraceResult;
	FHitResult DownwardTraceResult;
	// check if playing montage
	if (OwnerCharacter->GetCurrentMontage())
	{
		return false;
	}
	// Step1: Forward Wall Check
	const FVector ForwardStartLocation{ActorLocation.X, ActorLocation.Y, ActorLocation.Z + CapsuleHalfHeight};
	FVector ForwardTraceVector = UKismetMathLibrary::GetForwardVector(OwnerCharacter->GetActorRotation());
	ForwardTraceVector.X *= ReachDistance;
	ForwardTraceVector.Y *= ReachDistance;
	FVector ForwardEndLocation = ForwardStartLocation + ForwardTraceVector;
	FCollisionQueryParams Params{NAME_None, false, GetOwner()};
	bool bForwardHit = GetWorld()->SweepSingleByChannel(ForwardTraceResult, ForwardStartLocation, ForwardEndLocation,
									FQuat::Identity, ClimbTraceChannel,
									FCollisionShape::MakeCapsule(TraceCapsuleRadius, ForwardTraceCapsuleHalfHeight), Params);
	UPrimitiveComponent* ForwardHitPrimitive = ForwardTraceResult.GetComponent();
	if (!bForwardHit || ForwardHitPrimitive == nullptr || !ForwardHitPrimitive->CanCharacterStepUp(OwnerCharacter))
	{
		return false;
	}
	ForwardTraceNormal = -ForwardTraceResult.ImpactNormal;
	ForwardTraceNormal.Normalize();
	ForwardImpactPoint = ForwardTraceResult.ImpactPoint;
	if (UXCharacterMovementComponent* UxCharacterMovementComp = Cast<UXCharacterMovementComponent>(OwnerCharacter->GetCharacterMovement()))
	{
		UxCharacterMovementComp->SetWallNormal(-ForwardTraceNormal);	
	}
	// Set WallRight Direction
	WallRightDirection = FVector::CrossProduct(FVector::UpVector, ForwardTraceNormal).GetSafeNormal();
	WallRightDirection.Normalize();
	// Step2: Downward Trace
	const FVector ForwardTargetDirection{-ForwardTraceResult.ImpactNormal.GetSafeNormal2D()};
	const FVector TargetLocationOffset{TraceSetting.TargetLocationOffset * ForwardTargetDirection * CapsuleScale}; // 检测点沿墙壁内部偏移一段距离
	const FVector DownwardStartLocation{
		ForwardTraceResult.ImpactPoint.X + TargetLocationOffset.X,
			ForwardTraceResult.ImpactPoint.Y + TargetLocationOffset.Y,
			 ForwardTraceResult.ImpactPoint.Z + LedgeHeightDelta};
	const FVector DownwardEndLocation{
		DownwardStartLocation.X,
		DownwardStartLocation.Y,
		CapsuleBottomLocation.Z + TraceSetting.LedgeHeight.GetMin() * CapsuleScale};
	bool bDownwardHit = GetWorld()->SweepSingleByChannel(DownwardTraceResult, DownwardStartLocation, DownwardEndLocation,
												FQuat::Identity, ClimbTraceChannel,
												FCollisionShape::MakeCapsule(TraceCapsuleRadius, ForwardTraceCapsuleHalfHeight), Params);
	UPrimitiveComponent* DownwardHitPrimitive = DownwardTraceResult.GetComponent();
	UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
	DownwardImpactPoint = DownwardTraceResult.ImpactPoint;
	if (!bDownwardHit || DownwardHitPrimitive == nullptr
		|| !DownwardHitPrimitive->CanCharacterStepUp(OwnerCharacter)
		|| !MovementComp
		|| !MovementComp->IsWalkable(DownwardTraceResult))
	{
		return false;
	}
	// Step3: 判断墙壁上表面离玩家臀部距离是否过大
	static const FName PelvisSocket("PelvisSocket", FNAME_Find);
	const FVector DownwardHitImpactPoint{DownwardTraceResult.ImpactPoint};
	const float LedgeThreshold{TraceSetting.ClimbLedgeThreshold};
	float DistanceHipToSurface = DownwardHitImpactPoint.Z - OwnerCharacter->GetMesh()->GetSocketLocation(PelvisSocket).Z;
	// 玩家臀部距上表面超过阈值则无法攀爬
	bool bCanHang = DistanceHipToSurface < LedgeThreshold;
	return bCanHang;
}

void UClimbComponent::GrabLedge()
{
	ensure(OwnerCharacter);
	ensure(OwnerCharacter->GetCharacterMovement());
	ensure(OwnerCharacter->GetCapsuleComponent());
	
	const float CapsuleRadius{OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius()};
	const float CapsuleHalfHeight{OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()};
	// todo: 手部最好使用IK以贴合表面 
	const FVector TargetLocation{
		ForwardImpactPoint.X - ForwardTraceNormal.X * CapsuleRadius,
		ForwardImpactPoint.Y - ForwardTraceNormal.Y * CapsuleRadius,
		DownwardImpactPoint.Z - CapsuleHalfHeight + 25.f
	};
	const FRotator TargetRotator = UKismetMathLibrary::MakeRotFromXZ(ForwardTraceNormal, FVector::UpVector);
	FLatentActionInfo LatentActionInfo;
	LatentActionInfo.CallbackTarget = this;
	LatentActionInfo.ExecutionFunction = FName("OnMoveCompoComplete");
	LatentActionInfo.UUID = FMath::Rand();
	LatentActionInfo.Linkage = 0;
	UKismetSystemLibrary::MoveComponentTo(OwnerCharacter->GetRootComponent(), TargetLocation, TargetRotator,
		false, false, 0.2f, false, EMoveComponentAction::Move, LatentActionInfo);
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

void UClimbComponent::SetHangState(bool NewState)
{
	if (bIsHanging != NewState)
	{
		bIsHanging = NewState;
		OnHangStateChanged.Broadcast(bIsHanging);
	}
}

void UClimbComponent::OnMoveCompoComplete()
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
