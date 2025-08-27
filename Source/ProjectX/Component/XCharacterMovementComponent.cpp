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
	// Leave Hanging State
	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == static_cast<uint8>(ECustomMoveMode::CMove_Hanging))
	{
		bUseControllerDesiredRotation = true;
		OnHangStateChanged.Broadcast(bWantsToHanging);
		HangMoveInput = 0.f;
		WallRightDirection = FVector::ZeroVector;
		GravityScale = 1.f;
	}
	// Enter Hanging State
	if (MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECustomMoveMode::CMove_Hanging))
	{
		GravityScale = 0.f;
		Velocity = FVector::ZeroVector;
		OnHangStateChanged.Broadcast(bWantsToHanging);
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
	// SubStep
	float remainingTime = deltaTime;
	const EMovementMode StartingMovementMode = MovementMode;
	const uint8 StartingCustomMovementMode = CustomMovementMode;
	
	while (remainingTime >= MIN_TICK_TIME && Iterations < MaxSimulationIterations)
	{
		Iterations++;
		const float TimeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= TimeTick;

		RestorePreAdditiveRootMotionVelocity();
		const FVector Tangent = WallRightDirection.GetSafeNormal();
		
		if (!HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{
			FVector InputAccel = ScaleInputAcceleration((HangMoveInput * WallRightDirection).GetClampedToMaxSize(1.f));
			Acceleration = Tangent * FVector::DotProduct(InputAccel, Tangent);
			CalcVelocity(TimeTick, HangFriction, false, HangBraking);
			
			// ReProject
			Velocity = Tangent * FVector::DotProduct(Velocity, Tangent);
			
			// clamp 
			const float SpeedAlongTangent = FVector::DotProduct(Velocity, Tangent);
			Velocity = Tangent * FMath::Clamp(SpeedAlongTangent, -MaxHangSpeed, MaxHangSpeed);
		} else
		{
			// Override by RootMotion, keep velocity along tangent
			const float Along = FVector::DotProduct(Velocity, Tangent);		// cos
			Velocity = Tangent * Along;
		}
		
		ApplyRootMotionToVelocity(TimeTick);
		if (StartingMovementMode != MovementMode || StartingCustomMovementMode != CustomMovementMode)
		{
			// ApplyRootMotionToVelocity 可能改变MovementMode
			// 目前没有应用移动所以 TimeTick 和 Iterations 都复原
			StartNewPhysics(remainingTime + TimeTick, Iterations-1);
			return;
		}
		
		FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FVector Adjust = Velocity * TimeTick;
		
		FHitResult Hit(1.f);
		SafeMoveUpdatedComponent(Adjust, UpdatedComponent->GetComponentQuat(), true, Hit);
		if (Hit.Time < 1.f)
		{
			HandleImpact(Hit, TimeTick, Adjust);
		}
		if (!Adjust.IsNearlyZero())
		{
			// Check if there is still a wall to hang on
			FHitResult FrontHit, DownHit;
			const FVector Offset = Adjust.GetSafeNormal2D() * CapRadius();
			if (!CanHang(FrontHit, DownHit, Offset))
			{
				// Fall from the wall
				bWantsToHanging = false;
				SetMovementMode(MOVE_Falling);
				//StartNewPhysics(deltaTime, Iterations);
				return;
			}
		}
		
		
		if (!Hit.bBlockingHit && !bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity())
		{
			Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / TimeTick;
		}
		MaintainHorizontalHangingVelocity();
	}
}

float UXCharacterMovementComponent::GetMaxSpeed() const
{
	if (!CharacterOwner)
	{
		return Super::GetMaxSpeed();
	}
	if (MovementMode == MOVE_Walking)
	{
		if (bWantsToSprint)
		{
			return MaxSprintSpeed;
		}
		if (bWantsToRun)
		{
			return MaxRunSpeed;
		}
		return MaxWalkSpeed;
	}
	if (MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECustomMoveMode::CMove_Hanging))
	{
		return MaxHangSpeed;
	}
	return Super::GetMaxSpeed();
}

float UXCharacterMovementComponent::GetMaxAcceleration() const
{
	if (MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECustomMoveMode::CMove_Hanging))
	{
		return MaxHangAcceleration;
	}
	return Super::GetMaxAcceleration();
}

float UXCharacterMovementComponent::GetMinAnalogSpeed() const
{
	if (MovementMode == MOVE_Custom && CustomMovementMode == static_cast<uint8>(ECustomMoveMode::CMove_Hanging))
	{
		return MaxHangSpeed;
	}
	return Super::GetMinAnalogSpeed();
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
				TransitionRMS_ID = INDEX_NONE;
				SetWantsToHanging(true);
				SetMovementMode(MOVE_Custom, static_cast<uint8>(ECustomMoveMode::CMove_Hanging));
				bOrientRotationToMovement = false;
				// todo: 旋转改变不平滑。
				OwnerCharacter->GetController()->SetControlRotation(OwnerCharacter->GetActorRotation());
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
	// try to capture root motion source status
	// UpdateCharacterAfterMovement适合获取状态信息，但不适合进行动画/状态的切换(会影响当前帧)
	if (GetRootMotionSourceByID(TransitionRMS_ID) && GetRootMotionSourceByID(TransitionRMS_ID)->Status.HasFlag(ERootMotionSourceStatusFlags::Finished))
	{
		XLOG("Transition RMS Finished!!!");
		RemoveRootMotionSourceByID(TransitionRMS_ID);
		bWantsTransitionFinished = true;
	}
}

void UXCharacterMovementComponent::MaintainHorizontalHangingVelocity()
{
	if (GetGravitySpaceZ(Velocity) != 0.f)
	{
		Velocity = ProjectToGravityFloor(Velocity);
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
	FHitResult FrontHit, DownHit;
	if (!CanHang(FrontHit, DownHit))
	{
		return false;
	}
	
SLOG("Can Mantle/Climb/Hang");
	// Grab Ledge
	// todo: review this
	//const float MontageDuration = Montage_BracedHang->GetPlayLength();
	const float PlayRate = 1.f;
	bool bUseRelative = true;	// todo: 非移动物体使用World可避免计算节省性能
	
	TransitionRMS.Reset();
	TransitionRMS = MakeShared<FRootMotionSource_Hang>();
	TransitionRMS->AccumulateMode = ERootMotionAccumulateMode::Override;
	
	TransitionRMS->Montage = Montage_BracedHang;
	TransitionRMS_Name = "Hang";	// todo: do not hard cord TransitionRMS_Name
	
	// todo: Duration need to match with montage
	ensure(TransitionRMS && TransitionRMS->Montage);
	TransitionRMS->Duration = TransitionRMS->Montage->GetPlayLength();
	TransitionRMS->TargetPrimitive = DownHit.GetComponent();

	const auto StartWorldTransform {UpdatedComponent->GetComponentTransform()};
	TransitionRMS->StartWorldLocation = StartWorldTransform.GetLocation();
	TransitionRMS->StartWorldRotation = StartWorldTransform.GetRotation().Rotator();
	
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
	TransitionRMS->RelativeTargetLocation = RelativeTargetTransform.GetLocation();
	TransitionRMS->RelativeTargetRotation = RelativeTargetTransform.GetRotation().Rotator();
	// Apply Transition RootMotionSource
	bUseControllerDesiredRotation =false;
	Velocity = FVector::ZeroVector;
	TransitionRMS_ID = ApplyRootMotionSource(TransitionRMS);
	OwnerCharacter->GetMesh()->GetAnimInstance()->Montage_Play(TransitionRMS->Montage);
	XLOG("Playing Montage");
	return true;
}

bool UXCharacterMovementComponent::CanHang(FHitResult& OutFrontHit, FHitResult& OutDownHit, FVector Offset)
{
	ensure(OwnerCharacter && OwnerCharacter->GetMesh());
	ensure(OwnerCharacter->GetCharacterMovement() && OwnerCharacter->GetCapsuleComponent());
	
	// (Walking && !Crouch) || Flying || Falling || Custom::Hang(Edge Check) can continue
	if (!(IsMovementMode(MOVE_Walking) && !IsCrouching()) && !IsMovementMode(MOVE_Flying) && !IsMovementMode(MOVE_Falling) && !IsMovementMode(MOVE_Custom))
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
	const FVector ActorLocation{UpdatedComponent->GetComponentLocation() + Offset};
	const FVector Fwd = UpdatedComponent->GetForwardVector().GetSafeNormal2D();
	FCollisionQueryParams Params = OwnerCharacter->GetIgnoreCharacterParams();
	
	// Step1: Trace Front Face
	const FVector FrontStart{ActorLocation + FVector::UpVector * MaxStepHeight};
	const FVector FrontEnd = FrontStart + Fwd * ReachDistance;
	bool IsFrontHit = GetWorld()->SweepSingleByChannel(OutFrontHit, FrontStart, FrontEnd,
									FQuat::Identity, ClimbTraceChannel,
									FCollisionShape::MakeCapsule(CapRadius(), ForwardTraceCapsuleHalfHeight), Params);
LINE(FrontStart, FrontEnd, FColor::White);
	if (!IsFrontHit || !OutFrontHit.GetComponent() || !OutFrontHit.GetComponent()->CanCharacterStepUp(OwnerCharacter))
	{
SLOG("Front Trace Missed");
		return false;
	}
	const float FrontWallSteepnessAngle =
		FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FVector::UpVector, OutFrontHit.ImpactNormal.GetSafeNormal())));
	if (FMath::Abs(90 - FrontWallSteepnessAngle) > WallAngleThreshold)
	{
SLOG("Not a Vertical Wall");
		return false;
	}
	
	// Set WallRight Direction
	WallRightDirection = FVector::CrossProduct(FVector::UpVector, -OutFrontHit.ImpactNormal.GetSafeNormal()).GetSafeNormal();
	
	// Step2: Downward Trace
	const FVector FrontWallOffset = -OutFrontHit.ImpactNormal.GetSafeNormal2D() * LedgeOffset;	 

	const FVector DownStart = OutFrontHit.ImpactPoint + FrontWallOffset + FVector::UpVector * CapHalfHeight() * 2.f;
	const FVector DownEnd = OutFrontHit.ImpactPoint + FrontWallOffset + FVector::DownVector * CapHalfHeight();

	// todo: Capsule Half Height 应该 和 LedgeHeight有关
	bool bDownwardHit = GetWorld()->LineTraceSingleByChannel(OutDownHit, DownStart, DownEnd, ClimbTraceChannel, Params);

	UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
	const float LedgeSurfaceAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(FVector::UpVector, OutDownHit.ImpactNormal.GetSafeNormal())));
	
	if (FMath::Abs(LedgeSurfaceAngle) > WallAngleThreshold)
	{
SLOG("Not a Horizontal Surface");
		return false;
	}
	
	if (!bDownwardHit || !OutDownHit.GetComponent() || !OutDownHit.GetComponent()->CanCharacterStepUp(OwnerCharacter)
		|| !MovementComp || !MovementComp->IsWalkable(OutDownHit))
	{
SLOG("Downward Trace Missed");
		return false;
	}

	// Step3: 攀爬上方是否足够容纳玩家
	FVector ClearCapLoc = OutDownHit.ImpactPoint + (-OutDownHit.ImpactNormal.GetSafeNormal2D() * CapRadius()) + FVector::UpVector * (CapHalfHeight() + LedgeOffset);
	if (GetWorld()->OverlapAnyTestByChannel(ClearCapLoc, FQuat::Identity, ECollisionChannel::ECC_Visibility, FCollisionShape::MakeCapsule(CapRadius(), CapHalfHeight()), Params)) {
SLOG("No Room for Climb");
CAPSULE(ClearCapLoc, CapHalfHeight(), CapRadius(), FColor::Red, 10);
		return false;
	}
CAPSULE(ClearCapLoc, CapHalfHeight(), CapRadius(), FColor::Green, 10);
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
	return MantleStart;
}

void UXCharacterMovementComponent::SetHangInput(float Input)
{
	HangMoveInput = Input;
}

void UXCharacterMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
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
