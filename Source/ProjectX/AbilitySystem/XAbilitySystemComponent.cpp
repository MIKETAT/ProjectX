#include "XAbilitySystemComponent.h"

#include "Animation/XAnimInstance.h"

void UXAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	if (UXAnimInstance* Anim = Cast<UXAnimInstance>(ActorInfo->GetAnimInstance()))
	{
		Anim->InitializeWithAbilitySystemComponent(this);
	}
}
