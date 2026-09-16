// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Hritish Duvvur

#include "GameplayAbilitySystem/AttributeSets/AS_BaseAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"

void UAS_BaseAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UAS_BaseAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAS_BaseAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAS_BaseAttributeSet, CinderPool, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAS_BaseAttributeSet, MaxCinderPool, COND_None, REPNOTIFY_Always)
}

/// <summary>
/// This effects the attribute before it changes. This is usually used to clamp the values. Read the Unreal Documentation for more info. 
/// </summary>
void UAS_BaseAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	
	if (Attribute == GetCinderPoolAttribute())
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxCinderPool());
}

/// <summary>
/// Called just after a GameplayEffect is executed to modify the base value of an attribute.
/// </summary>
void UAS_BaseAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		// Clamp Health after effect modifies it
		SetHealth(GetHealth());
	}
	else if (Data.EvaluatedData.Attribute == GetCinderPoolAttribute())
	{
		SetCinderPool(GetCinderPool());
	}
}

void UAS_BaseAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAS_BaseAttributeSet, Health, OldHealth);
}

void UAS_BaseAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAS_BaseAttributeSet, MaxHealth, OldMaxHealth);
}

void UAS_BaseAttributeSet::OnRep_CinderPool(const FGameplayAttributeData& OldCinderPool) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAS_BaseAttributeSet, CinderPool, OldCinderPool)
}

void UAS_BaseAttributeSet::OnRep_MaxCinderPool(const FGameplayAttributeData& OldMaxCinderPool) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAS_BaseAttributeSet, MaxCinderPool, OldMaxCinderPool)
}

void UAS_BaseAttributeSet::OnRep_Poise(const FGameplayAttributeData& OldPoise) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAS_BaseAttributeSet, Poise, OldPoise);
}

void UAS_BaseAttributeSet::OnRep_MaxPoise(const FGameplayAttributeData& OldMaxPoise) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAS_BaseAttributeSet, MaxPoise, OldMaxPoise);
}

void UAS_BaseAttributeSet::OnRep_LightAttackDamage(const FGameplayAttributeData& OldLightAttackDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAS_BaseAttributeSet, LightAttackDamage, OldLightAttackDamage);
}

void UAS_BaseAttributeSet::OnRep_HeavyAttackDamage(const FGameplayAttributeData& OldHeavyAttackDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAS_BaseAttributeSet, HeavyAttackDamage, OldHeavyAttackDamage);
}

void UAS_BaseAttributeSet::OnRep_RangedAttackDamage(const FGameplayAttributeData& OldRangedAttackDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAS_BaseAttributeSet, RangedAttackDamage, OldRangedAttackDamage);
}