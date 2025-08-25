#include "XCharacterMovementComponent.h"
#include "ClimbComponent.h"
#include "ProjectX.h"
#include "Animation/RootMotionSource_Hang.h"
#include "Character/CharacterBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Settings/ClimbSettings.h"

void UXCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	// 从Hanging状态切换出去后恢复使用Controller的Rotation
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == static_cast<uint8>(ECustomMoveMode::CMove_Hanging))
	{
		bUseControllerDesiredRotation = true;
		OnHangStateChanged.Broadcast(bWantsToHanging);
	}
	if (MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECustomMoveMode::CMove_Hanging))
	{
		GravityScale = 0.f;
		Velocity = FVector::ZeroVector;
		OnHangStateChanged.Broadcast(bWantsToHanging);
	} else
	{
		if (MovementMode == MOVE_Walking)
		{
			GravityScale = 1.f;
		}
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
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}
	if (!CharacterOwner || (!CharacterOwner->GetController() && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}
	if (!WallRightDirection.IsNormalized())
	{
		return;
	}
	Iterations++;
	bJustTeleported = false;
	
	//RestorePreAdditiveRootMotionVelocity();
	//ApplyRootMotionToVelocity(deltaTime);

	// maybe we don't need to do substep in this
	Velocity = WallRightDirection * HangMoveInput * HangMoveSpeed;
	FHitResult HitResult;
	SafeMoveUpdatedComponent(Velocity * deltaTime, UpdatedComponent->GetComponentQuat(), true, HitResult);
}

float UXCharacterMovementComponent::GetMaxSpeed() const
{
	if (!CharacterOwner)
	{
		UE_LOG(LogTemp, Error, TEXT("UXCharacterMovementComponent::GetMaxSpeed: Owner is not a CharacterBase!"));
		return Super::GetMaxSpeed();
	}
	if (bWantsToRun)
	{
		return MaxRunSpeed;
	}
	if (bWantsToSprint)
	{
		return MaxSprintSpeed;
	}
	return Super::GetMaxSpeed();
}

void UXCharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (OwnerCharacter->bPressXJump)
	{
		XLOG("Try Climb");
		if (HangTracer())
		{
			XLOG("Hang Tracer Success");
			OwnerCharacter->StopJumping();
		} else
		{
			XLOG("Failed to Climb, Revert to Jump");
			OwnerCharacter->bPressXJump = false;
			CharacterOwner->bPressedJump = true;
			CharacterOwner->CheckJumpInput(DeltaSeconds);
		}
	}

	// Transition
	if (bWantsTransitionFinished)
	{
XLOG("Transition Finished");
		if (TransitionRMS_Name == "Hang")
		{
			if (TransitionRMS.IsValid())
			{
				TransitionMontage = nullptr;
				TransitionRMS_ID = INDEX_NONE;
				SetWantsToHanging(true);
				SetMovementMode(MOVE_Custom, static_cast<uint8>(ECustomMoveMode::CMove_Hanging));
				bOrientRotationToMovement = false;
				// todo: 旋转改变不平滑。
				OwnerCharacter->GetController()->SetControlRotation(OwnerCharacter->GetActorRotation());
XLOG("Successfully Set State to Hanging");;
			} else
			{
				SetMovementMode(MOVE_Walking);
			}
		}
		// Reset 
		TransitionRMS_Name = "";
		bWantsTransitionFinished = false;
	}
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UXCharacterMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
	/*if (!HasAnimRootMotion() && IsMovementMode(MOVE_Flying))
	{
		SetMovementMode(MOVE_Walking);
	}*/
	// try to capture root motion source status
	// UpdateCharacterAfterMovement适合获取状态信息，但不适合进行动画/状态的切换(会影响当前帧)
	if (GetRootMotionSourceByID(TransitionRMS_ID) && GetRootMotionSourceByID(TransitionRMS_ID)->Status.HasFlag(ERootMotionSourceStatusFlags::Finished))
	{
		XLOG("Transition Root Motion Source Finished!!!");
		RemoveRootMotionSourceByID(TransitionRMS_ID);
		bWantsTransitionFinished = true;
	}
}

bool UXCharacterMovementComponent::IsServer() const
{
	return CharacterOwner && CharacterOwner->HasAuthority();
}

float UXCharacterMovementComponent::CapRadius() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float UXCharacterMovementComponent::CapHalfHeight() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void UXCharacterMovementComponent::FXSavedMove::Clear()
{
	Super::Clear();
	bSavedPressXJump = 0;
	bSavedWantsToHanging = 0;
	bSavedWantsToRun = 0;
	bSavedWantsToSprint = 0;
	bSavedTransitionFinished = 0;
}

uint8 UXCharacterMovementComponent::FXSavedMove::GetCompressedFlags() const
{
	uint8 Result =  FSavedMove_Character::GetCompressedFlags(); // jump and crouch
	if (bSavedWantsToHanging)
	{
		Result |= FLAG_Custom_0;
	}
	if (bSavedWantsToRun)
	{
		Result |= FLAG_Custom_1;
	}
	if (bSavedWantsToSprint)
	{
		Result |= FLAG_Custom_2;
	}
	if (bSavedPressXJump)
	{
		Result |= FLAG_JumpPressed;
	}
	return Result;
}

// 返回能否合并(NewMove与当前是否一致)
bool UXCharacterMovementComponent::FXSavedMove::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter,
	float MaxDelta) const
{
	if (bSavedWantsToHanging != ((FXSavedMove*)&NewMove)->bSavedWantsToHanging)
	{
		return false;
	}
	if (bSavedWantsToRun != ((FXSavedMove*)&NewMove)->bSavedWantsToRun)
	{
		return false;
	}
	if (bSavedWantsToSprint != ((FXSavedMove*)&NewMove)->bSavedWantsToSprint)
	{
		return false;
	}
	if (bSavedPressXJump != ((FXSavedMove*)&NewMove)->bSavedPressXJump)
	{
		return false;
	}
	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UXCharacterMovementComponent::FXSavedMove::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);
	if (UXCharacterMovementComponent* CharMov = Cast<UXCharacterMovementComponent>(C->GetCharacterMovement()))
	{
		bSavedWantsToRun = CharMov->bWantsToRun;
		bSavedWantsToSprint = CharMov->bWantsToSprint;
		bSavedWantsToHanging = CharMov->bWantsToHanging;
		bSavedPressXJump = CharMov->OwnerCharacter->bPressXJump;
		bSavedTransitionFinished = CharMov->bWantsTransitionFinished;
	}
	Acceleration.X = FMath::RoundToFloat(Acceleration.X);
	Acceleration.Y = FMath::RoundToFloat(Acceleration.Y);
	Acceleration.Z = FMath::RoundToFloat(Acceleration.Z);
}

void UXCharacterMovementComponent::FXSavedMove::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);
	if (UXCharacterMovementComponent* CharMov = Cast<UXCharacterMovementComponent>(C->GetCharacterMovement()))
	{
		CharMov->OwnerCharacter->bPressXJump = bSavedPressXJump;
		CharMov->bWantsToHanging = bSavedWantsToHanging;
		CharMov->bWantsToRun = bSavedWantsToRun;
		CharMov->bWantsToSprint = bSavedWantsToSprint;
		CharMov->bWantsTransitionFinished = bSavedTransitionFinished;
	}
}

UXCharacterMovementComponent::FXNetworkPredictionData_Client_Character::FXNetworkPredictionData_Client_Character(
	const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
}

FSavedMovePtr UXCharacterMovementComponent::FXNetworkPredictionData_Client_Character::AllocateNewMove()
{
	return FSavedMovePtr(new FXSavedMove());
}

UXCharacterMovementComponent::UXCharacterMovementComponent(const FObjectInitializer& ObjInit)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UXCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);
	bWantsToHanging = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	bWantsToRun = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
	bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_2) != 0;
}

// todo
FNetworkPredictionData_Client* UXCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);
	if (!ClientPredictionData)
	{
		UXCharacterMovementComponent* MutableThis = const_cast<UXCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FXNetworkPredictionData_Client_Character(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}
	return ClientPredictionData;
}

bool UXCharacterMovementComponent::IsCrouching() const
{
	return OwnerCharacter && OwnerCharacter->bIsCrouched;
}

bool UXCharacterMovementComponent::HangTracer()
{
	ensure(OwnerCharacter && OwnerCharacter->GetMesh());
	ensure(OwnerCharacter->GetCharacterMovement() && OwnerCharacter->GetCapsuleComponent());
	
	// (Walking && !Crouch) || Flying can continue
	if (!(IsMovementMode(MOVE_Walking) && !IsCrouching()) && !IsMovementMode(MOVE_Flying))
	{
		return false;
	}
	if (OwnerCharacter->GetCurrentMontage())
	{
		return false;
	}
	
	// Settings Parameters
	const FClimbTraceSetting TraceSetting{OwnerCharacter->SelectClimbSetting()->TraceSetting};
	
	const float ReachDistance{TraceSetting.ReachDistance};
	const float LedgeOffset{TraceSetting.LedgeOffset};
	
	const float WallAngleThreshold{TraceSetting.WallAngleThreshold}; // 角度阈值
	const ECollisionChannel ClimbTraceChannel{TraceSetting.ClimbTraceChannel};
	
	const FVector2f LedgeHeight{TraceSetting.LedgeHeight};
	const float LedgeHeightDelta{TraceSetting.LedgeHeight.GetMax() - TraceSetting.LedgeHeight.GetMin()};
	const float ForwardTraceCapsuleHalfHeight{LedgeHeightDelta * 0.5f};
	
	// Parameters
	const FVector ActorLocation{UpdatedComponent->GetComponentLocation()};
	const FVector Fwd = UpdatedComponent->GetForwardVector().GetSafeNormal2D();
	
	FCollisionQueryParams Params = OwnerCharacter->GetIgnoreCharacterParams();
	
	// Trace Result Parameters
	FHitResult FrontHit;
	FHitResult DownHit;
	
	// Step1: Trace Front Face
	const FVector FrontStart{ActorLocation + FVector::UpVector * MaxStepHeight};
	const FVector FrontEnd = FrontStart + Fwd * ReachDistance;
/*CAPSULE(FrontStart, CapHalfHeight(), CapRadius(), FColor::White, 10);
CAPSULE(FrontEnd, CapHalfHeight(), CapRadius(), FColor::Yellow, 10);*/
	bool IsFrontHit = GetWorld()->SweepSingleByChannel(FrontHit, FrontStart, FrontEnd,
									FQuat::Identity, ClimbTraceChannel,
									FCollisionShape::MakeCapsule(CapRadius(), ForwardTraceCapsuleHalfHeight), Params);
LINE(FrontStart, FrontEnd, FColor::White);
	if (!IsFrontHit || !FrontHit.GetComponent() || !FrontHit.GetComponent()->CanCharacterStepUp(OwnerCharacter))
	{
		XLOG("Front Trace Missed");
		return false;
	}
	const float FrontWallSteepnessAngle =
		FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FVector::UpVector, FrontHit.ImpactNormal.GetSafeNormal())));
	if (FMath::Abs(90 - FrontWallSteepnessAngle) > WallAngleThreshold)
	{
		XLOG("Not a Vertical Wall");
		return false;
	}
	
//CAPSULE(FrontHit.ImpactPoint, CapHalfHeight(), CapRadius(), FColor::Green, 10);
//POINT(FrontHit.ImpactPoint, FColor::Red);
	
	
	// Set WallRight Direction
	WallRightDirection = FVector::CrossProduct(FVector::UpVector, -FrontHit.ImpactNormal.GetSafeNormal()).GetSafeNormal();
	
	// Step2: Downward Trace
	const FVector FrontWallOffset = -FrontHit.ImpactNormal.GetSafeNormal2D() * LedgeOffset;	 

	const FVector DownStart = FrontHit.ImpactPoint + FrontWallOffset + FVector::UpVector * CapHalfHeight() * 2.f;
	const FVector DownEnd = FrontHit.ImpactPoint + FrontWallOffset + FVector::DownVector * CapHalfHeight();

	// todo: Capsule Half Height 应该 和 LedgeHeight有关
	bool bDownwardHit = GetWorld()->LineTraceSingleByChannel(DownHit, DownStart, DownEnd, ClimbTraceChannel, Params);
	
LINE(DownStart, DownEnd, FColor::Yellow);

POINT(DownHit.ImpactPoint, FColor::Purple);

	UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
	const float LedgeSurfaceAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FVector::UpVector, DownHit.ImpactNormal.GetSafeNormal())));
	
	if (FMath::Abs(LedgeSurfaceAngle) > WallAngleThreshold)
	{
		XLOG("Not a Horizontal Surface");
		return false;
	}
	
	if (!bDownwardHit || !DownHit.GetComponent() || !DownHit.GetComponent()->CanCharacterStepUp(OwnerCharacter)
		|| !MovementComp || !MovementComp->IsWalkable(DownHit))
	{
		XLOG("Downward Trace Missed");
		return false;
	}

	// Step3: 攀爬上方是否足够容纳玩家
	FVector ClearCapLoc = DownHit.ImpactPoint + (-FrontHit.ImpactNormal.GetSafeNormal2D() * CapRadius()) + FVector::UpVector * (CapHalfHeight() + LedgeOffset);
	if (GetWorld()->OverlapAnyTestByChannel(ClearCapLoc, FQuat::Identity, ECollisionChannel::ECC_Visibility, FCollisionShape::MakeCapsule(CapRadius(), CapHalfHeight()), Params)) {
		XLOG("No Room for Climb");
CAPSULE(ClearCapLoc, CapHalfHeight(), CapRadius(), FColor::Red, 10);
		return false;
	}
CAPSULE(ClearCapLoc, CapHalfHeight(), CapRadius(), FColor::Green, 10);
	
	// Step4: 判断墙壁上表面离玩家臀部距离是否过大
	/*static const FName PelvisSocket("PelvisSocket", FNAME_Find);
	const FVector DownwardHitImpactPoint{DownHit.ImpactPoint};
	const float LedgeThreshold{TraceSetting.ClimbLedgeThreshold};
	float DistanceHipToSurface = DownwardHitImpactPoint.Z - OwnerCharacter->GetMesh()->GetSocketLocation(PelvisSocket).Z;
	// 玩家臀部距上表面超过阈值则无法攀爬
	bool CanHang = DistanceHipToSurface < LedgeThreshold;
	if (!CanHang)
	{
		return false;
	}*/
XLOG("Can Mantle/Climb/Hang");

	
	// Grab Ledge
	// todo: review this
	//const float MontageDuration = Montage_BracedHang->GetPlayLength();
	const float PlayRate = 1.f;
	bool bUseRelative = true;
	
	TransitionRMS.Reset();
	TransitionRMS = MakeShared<FRootMotionSource_Hang>();
	TransitionRMS->AccumulateMode = ERootMotionAccumulateMode::Override;
	TransitionRMS->Montage = Montage_BracedHang;	// todo:
	TransitionRMS_Name = "Hang";	// todo: do not hard cord TransitionRMS_Name
	
	// todo: Duration need to match with montage
	TransitionRMS->Duration = 1.27f; // MontageDuration / PlayRate; todo
	TransitionRMS->TargetPrimitive = DownHit.GetComponent();

	const auto StartWorldTransform {UpdatedComponent->GetComponentTransform()};
	TransitionRMS->StartWorldTransform = StartWorldTransform;
	
	// Calc Relative Offset
	FVector WorldOffset = OwnerCharacter->GetActorTransform().TransformVector(HandRelativeToCapsule);
	FVector TargetWorldLoc = GetMantleStartLocation(FrontHit, DownHit) - WorldOffset;

CAPSULE(TargetWorldLoc, CapHalfHeight(), CapRadius(), FColor::Orange, 30);
	FRotator TargetWorldRot = (-FrontHit.ImpactNormal.GetSafeNormal2D()).Rotation();

LINE(TargetWorldLoc, TargetWorldLoc + TargetWorldRot.Vector() * 100.f, FColor::Cyan);
	const auto WorldTargetTransform {FTransform{TargetWorldRot, TargetWorldLoc}};	// * HandRelativeToCapsuleTransform.Inverse()
	const auto RelativeTargetTransform{
		WorldTargetTransform.GetRelativeTransform(TransitionRMS->TargetPrimitive->GetComponentTransform())
	};
	TransitionRMS->RelativeTargetTransform = RelativeTargetTransform;
	
	// Apply Transition RootMotionSource
	//bUseControllerDesiredRotation = false;	// todo:
	bOrientRotationToMovement = true;		// todo
	Velocity = FVector::ZeroVector;
	TransitionRMS_ID = ApplyRootMotionSource(TransitionRMS);
	OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Play(TransitionRMS->Montage);
	XLOG("Playing Montage");
	return true;
}

FVector UXCharacterMovementComponent::GetMantleStartLocation(FHitResult& FrontHit, FHitResult& DownHit) const
{
	ensure(OwnerCharacter && OwnerCharacter->SelectClimbSetting());
	float LedgeOffset = OwnerCharacter->SelectClimbSetting()->TraceSetting.LedgeOffset;
	FVector MantleStart = DownHit.ImpactPoint;
	MantleStart += FrontHit.ImpactNormal.GetSafeNormal2D() * (CapRadius() + LedgeOffset);
	MantleStart += FVector::UpVector * CapHalfHeight();
	const float DownDistance = 2.f * CapHalfHeight();
	MantleStart += FVector::DownVector * DownDistance;
	MantleStart += FVector::UpVector * 65.f; // todo: hard code
//CAPSULE(MantleStart, CapHalfHeight(), CapRadius(), FColor::Orange, 10.f);
	return MantleStart;
}

void UXCharacterMovementComponent::SetHangInput(float Input)
{
	HangMoveInput = Input;
}

void UXCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
	SetMovementMode(MOVE_Walking);
	OwnerCharacter = Cast<ACharacterBase>(GetOwner());
	ensure(OwnerCharacter);
}

bool UXCharacterMovementComponent::IsMovementMode(EMovementMode InMoveMode) const
{
	return MovementMode == InMoveMode; 
}

bool UXCharacterMovementComponent::IsCustomMovementMode(ECustomMoveMode InCustomMoveMode) const
{
	return MovementMode == MOVE_Custom && InCustomMoveMode == static_cast<ECustomMoveMode>(CustomMovementMode);
}

void UXCharacterMovementComponent::SetWantsToHanging(bool NewValue)
{
	if (bWantsToHanging == NewValue)
	{
		return;
	}
	bWantsToHanging = NewValue;
	OnHangStateChanged.Broadcast(bWantsToHanging);
}
