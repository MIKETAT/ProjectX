// Fill out your copyright notice in the Description page of Project Settings.


#include "XAnimInstance.h"

#include "AbilitySystem/XAbilitySystemComponent.h"
#include "Character/CharacterBase.h"
#include "AbilitySystemGlobals.h"
#include "Component/XCharacterMovementComponent.h"


void UXAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<ACharacterBase>(GetOwningActor());
	if (IsValid(OwnerCharacter))
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwnerCharacter, false))
		{
			// todo: 这里为啥要初始化？忘了
			InitializeWithAbilitySystemComponent(ASC);
		}
	}
}

void UXAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (!IsValid(OwnerCharacter))
	{
		OwnerCharacter = Cast<ACharacterBase>(GetOwningActor());
		return;
	}
	if (OwnerCharacter->CharMov)
	{
		IsHanging = OwnerCharacter->CharMov->bWantsToHanging != 0;	
	}
}

void UXAnimInstance::InitializeWithAbilitySystemComponent(UAbilitySystemComponent* ASC)
{
	check(ASC);
	GameplayTagPropertyMap.Initialize(this, ASC);
}

void UXAnimInstance::SetHangMoveDirection(EHangMoveDirection MoveDirection)
{
	HangMoveDirection = MoveDirection;
}
