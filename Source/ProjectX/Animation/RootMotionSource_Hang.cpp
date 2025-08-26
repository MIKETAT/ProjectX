// Fill out your copyright notice in the Description page of Project Settings.


#include "RootMotionSource_Hang.h"

#include "ProjectX.h"
#include "UProjectXUtils.h"
#include "GameFramework/Character.h"

FRootMotionSource_Hang::FRootMotionSource_Hang()
{
	Priority = 1000;
}

bool FRootMotionSource_Hang::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	if (!FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess))
	{
		bOutSuccess = false;
		return false;
	}
	bOutSuccess = true;
	auto bSuccessLocal{true};

	Ar << Montage;
	Ar << TargetPrimitive;

	bOutSuccess &= SerializePackedVector<100, 30>(StartWorldLocation, Ar);

	StartWorldRotation.NetSerialize(Ar, Map, bSuccessLocal);
	StartWorldRotation.Normalize();
	bOutSuccess &= bSuccessLocal;

	bOutSuccess &= SerializePackedVector<100, 30>(RelativeTargetLocation, Ar);

	RelativeTargetRotation.NetSerialize(Ar, Map, bSuccessLocal);
	RelativeTargetRotation.Normalize();
	bOutSuccess &= bSuccessLocal;

	return bOutSuccess;
}

FRootMotionSource* FRootMotionSource_Hang::Clone() const
{
	return new FRootMotionSource_Hang{*this};
}

bool FRootMotionSource_Hang::Matches(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::Matches(Other))
	{
		return false;
	}
	const auto* OtherHang = static_cast<const FRootMotionSource_Hang*>(Other);
	return Montage == OtherHang->Montage && TargetPrimitive == OtherHang->TargetPrimitive;
}

UScriptStruct* FRootMotionSource_Hang::GetScriptStruct() const
{
	return StaticStruct();
}

void FRootMotionSource_Hang::AddReferencedObjects(class FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(Collector);
	Collector.AddReferencedObject(Montage);
}

void FRootMotionSource_Hang::PrepareRootMotion(float SimulationTime, float MovementTickTime,
	const ACharacter& Character, const UCharacterMovementComponent& MoveComponent)
{
	RootMotionParams.Clear();
	
	if (GetDuration() <= UE_SMALL_NUMBER || MovementTickTime <= UE_SMALL_NUMBER || !TargetPrimitive.IsValid())
	{
		XLOG("Invalid RootMotion");
		RootMotionParams.Clear();
		return;
	}
	
	// current root motion time
	const auto MontageTime{GetTime() * Montage->RateScale};
	Character.GetMesh()->GetAnimInstance()->Montage_SetPosition(Montage, FMath::Max(0.f, MontageTime - MovementTickTime));
	
	// Get Current Target World Transform
	FTransform TargetWorldTransform = FTransform{RelativeTargetRotation, RelativeTargetLocation} * TargetPrimitive->GetComponentTransform();
	
	// Twist to only keep yaw
	const FQuat Twist{UProjectXUtils::GetTwist(TargetWorldTransform.GetRotation(), -Character.GetGravityDirection())};
	TargetWorldTransform.SetRotation(Twist);

	// Delta Location
	const float CurrentMoveFraction = GetTime() / GetDuration();
	const float ExpectMoveFraction = (GetTime() + SimulationTime) / Duration;
	FVector CurrentLocation = FMath::Lerp(StartWorldLocation, TargetWorldTransform.GetLocation(), CurrentMoveFraction);
	FVector ExpectLocation = FMath::Lerp(StartWorldLocation, TargetWorldTransform.GetLocation(), ExpectMoveFraction);
	FVector DeltaLocation = ExpectLocation - CurrentLocation;

	// Delta Rotation
	FQuat CurrentQuat = FQuat::Slerp(StartWorldRotation.Quaternion(), TargetWorldTransform.GetRotation(), CurrentMoveFraction);
	FQuat ExpectQuat = FQuat::Slerp(StartWorldRotation.Quaternion(), TargetWorldTransform.GetRotation(), ExpectMoveFraction);
	FQuat DeltaQuat = CurrentQuat.Inverse() * ExpectQuat;
	FTransform NewTransform{DeltaQuat, DeltaLocation / MovementTickTime};
	
	RootMotionParams.Set(NewTransform);
	SetTime(GetTime() + SimulationTime);
}
