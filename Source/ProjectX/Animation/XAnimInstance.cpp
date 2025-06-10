// Fill out your copyright notice in the Description page of Project Settings.


#include "XAnimInstance.h"
#include "Character/CharacterBase.h"

void UXAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!IsValid(Character))
	{
		return;
	}
	Gait = Character->GetGait();
}

void UXAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	Character = Cast<ACharacterBase>(GetOwningActor());
}
