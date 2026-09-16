// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur

#include "GameplayAbilitySystem/ASC_AbilitySystemComponent.h"
#include "Characters/PS_PlayerState.h"
#include "GameplayAbilitySystem/BFL_AbilitySystemLibrary.h"
#include "GameplayAbilitySystem/GA_EmberAbility.h"
#include "GameplayAbilitySystem/Data/DA_AbilityInfo.h"

void UASC_AbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &UASC_AbilitySystemComponent::EffectApplied);
}

void UASC_AbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbiliites)
{
	for (TSubclassOf<UGameplayAbility> Ability : StartupAbiliites)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Ability.GetDefaultObject(), 1);
		if (const UGA_EmberAbility* EmberAbility = Cast<UGA_EmberAbility>(Ability.GetDefaultObject()))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(EmberAbility->StartupInputTag);
			FGameplayTag DynamicTag = FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Equipped"));
			AbilitySpec.DynamicAbilityTags.AddTag(DynamicTag);
		}
		GiveAbility(AbilitySpec);
	}

	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();
}

void UASC_AbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (AbilitySpec.IsActive())
				InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, AbilitySpec.GetAbilityInstances().Last()->GetCurrentActivationInfoRef().GetActivationPredictionKey());
		}
	}
}

void UASC_AbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
				TryActivateAbility(AbilitySpec.Handle);
		}
	}
}

void UASC_AbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;
	
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.GetAbilityInstances().Last()->GetCurrentActivationInfoRef().GetActivationPredictionKey());
		}
	}
}

void UASC_AbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec))
			UE_LOG(LogTemp, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
	}
}

void UASC_AbilitySystemComponent::UpdateAbilityStatus(int32 Level)
{
	UDA_AbilityInfo* AbilityInfo = UBFL_AbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for (const FEmberAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		if (!Info.AbilityTag.IsValid()) continue;
		if (Level < Info.LevelRequirement) continue;
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			AbilitySpec.DynamicAbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Eligible")));
			GiveAbility(AbilitySpec);
			MarkAbilitySpecDirty(AbilitySpec);
			UE_LOG(LogTemp, Warning, TEXT("Gave ability %s with status %s"), *Info.AbilityTag.ToString(), *Info.StatusTag.ToString());
			ClientUpdateAbilityStatus(Info.AbilityTag, FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Eligible")), 1);
		}
	}
}

bool UASC_AbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription, FText& OutSpellName, FText& OutOathPointCost, FText& OutSpellCost)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		UGA_EmberAbility* EmberAbility = Cast<UGA_EmberAbility>(AbilitySpec->Ability.Get());
		if (EmberAbility)
		{
			OutDescription = EmberAbility->GetDescription(AbilitySpec->Level);
			OutSpellName = EmberAbility->GetDisplayName();
			OutOathPointCost = EmberAbility->GetOathPointCost();
			OutSpellCost = EmberAbility->GetCostOfUsage(AbilitySpec->Level);
			OutNextLevelDescription = EmberAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			return true;
		}
	}

	const UDA_AbilityInfo* AbilityInfo = UBFL_AbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("EmberAbility.Spells.None"))) || !AbilityInfo)
	{
		OutDescription = FString();
		OutSpellCost = FText();
		OutSpellName = FText();
	}
	else
	{
		OutDescription = UGA_EmberAbility::GetLockedDescription(AbilityInfo -> FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	}

	OutNextLevelDescription = FString();
	return false;
}

void UASC_AbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* AbilitySpec)
{
	const FGameplayTag Slot = GetInputTagFromSpec(*AbilitySpec);
	AbilitySpec -> DynamicAbilityTags.RemoveTag(Slot);
	MarkAbilitySpecDirty(*AbilitySpec);
}

void UASC_AbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& SlotTag)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(&AbilitySpec, SlotTag))
			ClearSlot(&AbilitySpec);
	}
}

bool UASC_AbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec* AbilitySpec, const FGameplayTag& SlotTag)
{
	for (FGameplayTag Tag : AbilitySpec->DynamicAbilityTags)
	{
		if (Tag.MatchesTagExact(SlotTag)) 
			return true;
	}
	return false;
}

FGameplayTag UASC_AbilitySystemComponent::GetAbilityFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("EmberAbility.Spells"))) || Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("EmberAbility.Core"))))
			{
				UE_LOG(LogTemp, Warning, TEXT("Found spell %s"), *Tag.ToString());
				return Tag;
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Unknown tag %s"), *Tag.ToString());
			}
		}
	}
	
	return FGameplayTag();
}

FGameplayTag UASC_AbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag Tag : AbilitySpec.DynamicAbilityTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
			return Tag;
	}
	return FGameplayTag();
}

FGameplayTag UASC_AbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag StatusTag : AbilitySpec.DynamicAbilityTags)
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
			return StatusTag;
	}
	return FGameplayTag();
}

FGameplayAbilitySpec* UASC_AbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get() -> AbilityTags)
		{
			if (Tag.MatchesTag(AbilityTag))
				return &AbilitySpec;
		}
	}
	return nullptr;
}

FGameplayTag UASC_AbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
		return GetStatusFromSpec(*AbilitySpec);
	
	return FGameplayTag();
}

FGameplayTag UASC_AbilitySystemComponent::GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
		return GetInputTagFromSpec(*AbilitySpec);
	
	return FGameplayTag();
}

void UASC_AbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& SlotTag)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const FGameplayTag& PrevSlotTag = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag StatusTag = GetStatusFromSpec(*AbilitySpec);
		
		const bool bStatusValid = StatusTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Unlocked"))) || StatusTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Equipped")));
		if (bStatusValid)
		{
			//Remove Input Tag from any ability that has it
			ClearAbilitiesOfSlot(SlotTag);
			
			//Clear this abilities slot
			ClearSlot(AbilitySpec);
			
			//Now assign this ability to this slot
			AbilitySpec->DynamicAbilityTags.AddTag(SlotTag);
			if (StatusTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Unlocked"))))
			{
				AbilitySpec->DynamicAbilityTags.RemoveTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Unlocked")));
				AbilitySpec->DynamicAbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Equipped")));
			}
			MarkAbilitySpecDirty(*AbilitySpec);
		}
		ClientEquipAbility(AbilityTag, FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Equipped")), SlotTag, PrevSlotTag);
	}
}

void UASC_AbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag);
	if (AbilitySpec)
	{
		AC_BasePlayer *Player = Cast<AC_BasePlayer>(GetAvatarActor());
		if (APS_PlayerState *PS = Cast<APS_PlayerState>(Player -> GetPlayerState()))
		{
			PS->AddSpellPoints(-1);
		}
		
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);
		if (Status.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Eligible"))))
		{
			AbilitySpec->DynamicAbilityTags.RemoveTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Eligible")));
			AbilitySpec->DynamicAbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Unlocked")));
			Status = FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Unlocked"));
		}
		else if (Status.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Equipped"))) || Status.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Unlocked"))))
		{
			AbilitySpec->Level += 1;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Ability %s is not eligible to spend points"), *AbilityTag.ToString());
		}
		
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

///Summary
/// Delegate handler for when a gameplay effect is applied to the owner of this Ability System Component.
void UASC_AbilitySystemComponent::EffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	
	EffectAssetTags.Broadcast(TagContainer);
}

void UASC_AbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	if (!bStartupAbilitiesGiven)
	{
		bStartupAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast();
	}
}

void UASC_AbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

void UASC_AbilitySystemComponent::ClientEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const FGameplayTag& SlotTag, const FGameplayTag& PrevSlotTag)
{
	AbilityEquipped.Broadcast(AbilityTag, StatusTag, SlotTag, PrevSlotTag);
}