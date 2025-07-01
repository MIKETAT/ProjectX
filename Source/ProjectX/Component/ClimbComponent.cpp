#include "Component/ClimbComponent.h"
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
	ensure(OwnerCharacter);
	if (!OwnerCharacter)
	{
		OwnerCharacter = Cast<ACharacterBase>(GetOwner());
	}
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
	if (IsValid(OwnerCharacter->GetCurrentMontage()))
	{
		return false;
	}

	// Step1: Forward Wall Check
	const FVector ForwardStartLocation = OwnerCharacter->GetActorLocation() + CapsuleHalfHeight;
	FVector ForwardTraceVector = UKismetMathLibrary::GetForwardVector(OwnerCharacter->GetActorRotation());
	ForwardTraceVector.X *= ReachDistance;
	ForwardTraceVector.Y *= ReachDistance;
	FVector ForwardEndLocation = ForwardStartLocation + ForwardTraceVector;
	//UKismetSystemLibrary::SphereTraceSingle(GetWorld(), StartLocation, EndLocation, SphereRadius, )
	
	FCollisionQueryParams Params{NAME_None, false, GetOwner()};
	bool bForwardHit = GetWorld()->SweepSingleByChannel(ForwardTraceResult, ForwardStartLocation, ForwardEndLocation,
												FQuat::Identity, ClimbTraceChannel,
												FCollisionShape::MakeCapsule(TraceCapsuleRadius, ForwardTraceCapsuleHalfHeight), Params);
	UPrimitiveComponent* ForwardHitPrimitive = ForwardTraceResult.GetComponent();
	if (!bForwardHit || !IsValid(ForwardHitPrimitive) || !ForwardHitPrimitive->CanCharacterStepUp(OwnerCharacter))
	{
		return false;
	}
	// Step2: Downward Trace
	const FVector ForwardTargetDirection{-ForwardTraceResult.ImpactNormal.GetSafeNormal2D()};
	const FVector TargetLocationOffset{TraceSetting.TargetLocationOffset * ForwardTargetDirection * CapsuleScale}; // 检测点沿墙壁内部偏移一段距离
	
	const FVector DownwardStartLocation{
		ForwardTraceResult.ImpactPoint.X + TargetLocationOffset.X,
			ForwardTraceResult.ImpactPoint.Y + TargetLocationOffset.Y,
			CapsuleBottomLocation.Z + LedgeHeightDelta};
	const FVector DownwardEndLocation{
		DownwardStartLocation.X,
		DownwardStartLocation.Y,
		CapsuleBottomLocation.Z + TraceSetting.LedgeHeight.GetMin() * CapsuleScale};
	
	bool bDownwardHit = GetWorld()->SweepSingleByChannel(DownwardTraceResult, DownwardStartLocation, DownwardEndLocation,
												FQuat::Identity, ClimbTraceChannel,
												FCollisionShape::MakeCapsule(TraceCapsuleRadius, ForwardTraceCapsuleHalfHeight), Params);
	UPrimitiveComponent* DownwardHitPrimitive = DownwardTraceResult.GetComponent();
	if (!bDownwardHit || !IsValid(DownwardHitPrimitive)
		|| !DownwardHitPrimitive->CanCharacterStepUp(OwnerCharacter)
		|| !OwnerCharacter->GetCharacterMovement()->IsWalkable(DownwardTraceResult))
	{
		return false;
	}
	
	// Step3: 判断墙壁上表面离玩家臀部距离是否过大
	const FName PelvisSocket{"PelvisSocket"};
	const FVector DownwardHitImpactPoint{DownwardTraceResult.ImpactPoint};
	const float LedgeThreshold{TraceSetting.ClimbLedgeThreshold};
	float DistanceHipToSurface = DownwardHitImpactPoint.Z - OwnerCharacter->GetMesh()->GetSocketLocation(PelvisSocket).Z;
	// 玩家臀部距上表面超过阈值则无法攀爬
	return DistanceHipToSurface > LedgeThreshold;
}

