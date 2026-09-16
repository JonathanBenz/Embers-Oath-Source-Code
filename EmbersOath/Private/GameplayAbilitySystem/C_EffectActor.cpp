// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contibutors: Jonathan Benz

#include "GameplayAbilitySystem/C_EffectActor.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

AC_EffectActor::AC_EffectActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AC_EffectActor::BeginPlay()
{
	Super::BeginPlay();
}

void AC_EffectActor::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	check(GameplayEffectClass);
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (!TargetASC) return;

	FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());

	const bool bIsInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy == EGameplayEffectDurationType::Infinite;
	if (bIsInfinite && InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		const uint32 UID = TargetActor->GetUniqueID();
		ActiveInfiniteEffects.Add(UID, ActiveEffectHandle);
	}

	if (bDestroyOnEffectRemoval) Destroy();
}

void AC_EffectActor::OnOverlap(AActor* TargetActor)
{
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);

	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);

	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
}

void AC_EffectActor::OnEndOverlap(AActor* TargetActor)
{
	if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
		ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);

	if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
		ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);

	if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
		ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);

	// --- Remove Infinite Effect If We Have One
	if (InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
	{
		const uint32 UID = TargetActor->GetUniqueID();
		if (!ActiveInfiniteEffects.Contains(UID)) return;

		const FActiveGameplayEffectHandle& EffectHandle = ActiveInfiniteEffects[UID];
		if (!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor)->RemoveActiveGameplayEffect(EffectHandle, 1)) return;

		ActiveInfiniteEffects.Remove(UID);
	}
}