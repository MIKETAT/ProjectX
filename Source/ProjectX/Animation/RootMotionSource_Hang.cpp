// Fill out your copyright notice in the Description page of Project Settings.


#include "RootMotionSource_Hang.h"

#include "ProjectX.h"
#include "UProjectXUtils.h"
#include "GameFramework/Character.h"

FRootMotionSource_Hang::FRootMotionSource_Hang()
{
	Priority = 100;
}

bool FRootMotionSource_Hang::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	return FRootMotionSource::NetSerialize(Ar, Map, bOutSuccess);
}

FRootMotionSource* FRootMotionSource_Hang::Clone() const
{
	return new FRootMotionSource_Hang{*this};
}

bool FRootMotionSource_Hang::Matches(const FRootMotionSource* Other) const
{
	return FRootMotionSource::Matches(Other);
}

UScriptStruct* FRootMotionSource_Hang::GetScriptStruct() const
{
	return StaticStruct();
}

void FRootMotionSource_Hang::AddReferencedObjects(class FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(Collector);
	// todo: 
	//Collector.AddReferencedObjects()
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
	
	// Get Target World Transform
	FTransform TargetWorldTransform = RelativeTargetTransform * TargetPrimitive->GetComponentTransform();
	
	// Twist to only keep yaw
	const FQuat Twist{UProjectXUtils::GetTwist(TargetWorldTransform.GetRotation(), -Character.GetGravityDirection())};
	TargetWorldTransform.SetRotation(Twist);

	// Delta Location
	const float CurrentMoveFraction = GetTime() / GetDuration();
	const float ExpectMoveFraction = (GetTime() + SimulationTime) / Duration;
	FVector CurrentLocation = FMath::Lerp(StartWorldTransform.GetLocation(), TargetWorldTransform.GetLocation(), CurrentMoveFraction);
	FVector ExpectLocation = FMath::Lerp(StartWorldTransform.GetLocation(), TargetWorldTransform.GetLocation(), ExpectMoveFraction);
	FVector DeltaLocation = ExpectLocation - CurrentLocation;

	// Delta Rotation
	FQuat CurrentQuat = FQuat::Slerp(StartWorldTransform.GetRotation(), TargetWorldTransform.GetRotation(), CurrentMoveFraction);
	FQuat ExpectQuat = FQuat::Slerp(StartWorldTransform.GetRotation(), TargetWorldTransform.GetRotation(), ExpectMoveFraction);
	FQuat DeltaQuat = CurrentQuat.Inverse() * ExpectQuat;
	FTransform NewTransform{DeltaQuat, DeltaLocation / MovementTickTime};
	
	RootMotionParams.Set(NewTransform);
	SetTime(GetTime() + SimulationTime);
}
