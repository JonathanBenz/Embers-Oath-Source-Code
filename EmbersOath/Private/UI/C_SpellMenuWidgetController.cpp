// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur, Ayushi Parikh

#include "UI/C_SpellMenuWidgetController.h"
#include "Characters/PS_PlayerState.h"
#include "GameplayAbilitySystem/ASC_AbilitySystemComponent.h"
#include "GameplayAbilitySystem/BFL_AbilitySystemLibrary.h"
#include "GameplayAbilitySystem/Data/DA_AbilityInfo.h"

void UC_SpellMenuWidgetController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();
}

void UC_SpellMenuWidgetController::BindCallbacksToDependencies()
{
	GetEmberAbilitySystemComponent()->AbilityStatusChanged.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 NewLevel)
	{
		if (SelectedAbility.AbilityTag.MatchesTagExact(AbilityTag))
		{
			SelectedAbility.StatusTag = StatusTag;
			bool bSpendPointsButtonEnabled = false;
			bool bEquipButtonEnabled = false;
			ShouldEnableButtons(StatusTag, CurrentSpellPoints, bSpendPointsButtonEnabled, bEquipButtonEnabled);
			FString DescriptionString;
			FString NextLevelDescription;
			FText SpellName;
			FText OathPointCost;
			FText SpellCost;
			GetEmberAbilitySystemComponent()->GetDescriptionsByAbilityTag(AbilityTag, DescriptionString, NextLevelDescription, SpellName, OathPointCost, SpellCost);
			SpellGlobeSelectedDelegate.Broadcast(bSpendPointsButtonEnabled, bEquipButtonEnabled, DescriptionString, NextLevelDescription, SpellName, OathPointCost, SpellCost);
		}
		if (AbilityInfo)
		{
			FEmberAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
			Info.StatusTag = StatusTag;
			AbilityInfoDelegate.Broadcast(Info);
			OnSpellGlobeSelectedPassAssociatedAbilityDelegate.Broadcast(Info);

		}
	});
	
	GetEmberAbilitySystemComponent()->AbilityEquipped.AddUObject(this, &UC_SpellMenuWidgetController::OnAbilityEquipped);
}

void UC_SpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	if (bWaitingForEquipSelection)
	{
		FGameplayTag SelectedAbilityTag = AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityTag;
		StopWaitForEquipSelectionDelegate.Broadcast(SelectedAbilityTag);	
		bWaitingForEquipSelection = false;
	}
	
	FGameplayTag AbilityStatus;
	
	bool bTagValid = AbilityTag.IsValid();
	bool BTagNone = AbilityTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("EmberAbility.Spells.None")));
	FGameplayAbilitySpec* AbilitySpec = GetEmberAbilitySystemComponent()->GetSpecFromAbilityTag(AbilityTag);
	const bool bSpecValid = AbilitySpec != nullptr;

	if (!bTagValid || BTagNone || !bSpecValid)
	{
		AbilityStatus = FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Locked"));
	}
	else
	{
		AbilityStatus = GetEmberAbilitySystemComponent()->GetStatusFromSpec(*AbilitySpec);
	}
	
	SelectedAbility.AbilityTag = AbilityTag;
	SelectedAbility.StatusTag = AbilityStatus;
	
	bool bSpendPointsButtonEnabled = false;
	bool bEquipButtonEnabled = false;
	ShouldEnableButtons(AbilityStatus, CurrentSpellPoints, bSpendPointsButtonEnabled, bEquipButtonEnabled);
	
	FString DescriptionString;
	FString NextLevelDescription;
	FText SpellName;
	FText OathPointCost;
	FText SpellCost;
	GetEmberAbilitySystemComponent()->GetDescriptionsByAbilityTag(SelectedAbility.AbilityTag, DescriptionString, NextLevelDescription, SpellName,OathPointCost, SpellCost);
	SpellGlobeSelectedDelegate.Broadcast(bSpendPointsButtonEnabled, bEquipButtonEnabled, DescriptionString, NextLevelDescription, SpellName, OathPointCost, SpellCost);

	OnSpellGlobeSelectedPassAssociatedAbilityDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(SelectedAbility.AbilityTag));
}

void UC_SpellMenuWidgetController::SpendPointButtonPressed()
{
	if (GetEmberAbilitySystemComponent())
	{
		GetEmberAbilitySystemComponent()->ServerSpendSpellPoint(SelectedAbility.AbilityTag);
	}
}

void UC_SpellMenuWidgetController::EquipButtonPressed()
{
	const FGameplayTag AbilityType = AbilityInfo -> FindAbilityInfoForTag(SelectedAbility.AbilityTag).AbilityTypeTag;
	WaitForEquipSelectionDelegate.Broadcast(AbilityType);
	bWaitingForEquipSelection = true;
	
	const FGameplayTag SelectedStatus = GetEmberAbilitySystemComponent() ->GetStatusFromAbilityTag(SelectedAbility.AbilityTag);
	if (SelectedStatus.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Equipped"))))
	{
		SelectedSlot = GetEmberAbilitySystemComponent()->GetInputTagFromAbilityTag(SelectedAbility.AbilityTag);
	}
}

void UC_SpellMenuWidgetController::GlobeDeselected()
{
	if (bWaitingForEquipSelection)
	{
		FGameplayTag SelectedAbilityTag = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.AbilityTag).AbilityTag;
		StopWaitForEquipSelectionDelegate.Broadcast(SelectedAbilityTag);
		bWaitingForEquipSelection = false;
	}
	
	SelectedAbility.AbilityTag = FGameplayTag::RequestGameplayTag(FName("EmberAbility.Spells.None"));
	SelectedAbility.StatusTag = FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Locked"));
	SpellGlobeSelectedDelegate.Broadcast(false, false, FString(), FString(), FText(), FText(), FText());
	
	OnSpellGlobeSelectedPassAssociatedAbilityDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(SelectedAbility.AbilityTag));
}

void UC_SpellMenuWidgetController::SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityTypeTag)
{
	if (!bWaitingForEquipSelection) return;
	//Check Selected ability against slots ability type (Both should be Either Offensive or Passive) and if it isnt the same, return
	const FGameplayTag SelectedAbilityTypeTag = AbilityInfo -> FindAbilityInfoForTag(SelectedAbility.AbilityTag).AbilityTypeTag;
	if (!SelectedAbilityTypeTag.MatchesTagExact(AbilityTypeTag)) return;
	GetEmberAbilitySystemComponent() -> ServerEquipAbility(SelectedAbility.AbilityTag, SlotTag);

	//Tell the spell menu we have actually equipped
	OnSpellEquippedForFirstTimeWBPDelegate.Broadcast();
}

//Broadcasts to SpellGlobes that are in Spell Menu
void UC_SpellMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const FGameplayTag& SlotTag, const FGameplayTag& PrevSlotTag)
{
	bWaitingForEquipSelection = false;
	
	FEmberAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Unlocked"));
	LastSlotInfo.InputTag = PrevSlotTag;
	LastSlotInfo.AbilityTag = FGameplayTag::RequestGameplayTag(FName("EmberAbility.Spells.None"));
	
	//Broadcast Empty info if previous slot is a valid slot, otherwise it means we are equipping from an empty slot so we dont need to update the old slot
	AbilityInfoDelegate.Broadcast(LastSlotInfo);
	
	FEmberAbilityInfo NewSlotInfo = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	NewSlotInfo.StatusTag = StatusTag;
	NewSlotInfo.InputTag = SlotTag;
	AbilityInfoDelegate.Broadcast(NewSlotInfo);

	StopWaitForEquipSelectionDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityTypeTag);
	SpellGlobeReassignedDelegate.Broadcast(AbilityTag);
	GlobeDeselected();
}

void UC_SpellMenuWidgetController::ReassignSpellGlobeToNextAvailableSlot(const FEmberAbilityInfo& AbilityInfoToReassign)
{
	FindNewSlotForEquippedSpellDelegate.Broadcast();
	FSelectedAbility Temp = SelectedAbility;
	
	//If you are trying to reassign the same ability, this function need not be called
	if (SelectedAbility.AbilityTag.MatchesTagExact(AbilityInfoToReassign.AbilityTag)) return;
	
	//If Ability tag is Valid and If Ability to Reassign is not None, then reassign Spell
	if (AbilityInfoToReassign.AbilityTag.IsValid() && (!AbilityInfoToReassign.AbilityTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("EmberAbility.Spells.None"))) || AbilityInfoToReassign.AbilityTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("EmberAbility.Core")))))
	{
		GetEmberAbilitySystemComponent()->ServerEquipAbility(AbilityInfoToReassign.AbilityTag, NextAvailableSlotTag);
		//Tell the spell menu we have actually equipped
		OnSpellEquippedForFirstTimeWBPDelegate.Broadcast();
	}
	SelectedAbility = Temp;
}

void UC_SpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bSpendPointsButtonEnabled, bool& bEquipButtonEnabled)
{
	bSpendPointsButtonEnabled = false;
	bEquipButtonEnabled = false;
	
	if (AbilityStatus == FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Equipped")))
	{
		bEquipButtonEnabled = true;
	}
	else if (AbilityStatus == FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Eligible")))
	{
		bSpendPointsButtonEnabled = SpellPoints > 0;
	}
	else if (AbilityStatus == FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Unlocked")))
	{
		bEquipButtonEnabled = true;
	}
	else
	{
		GEngine -> AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("***********Unknown ability status tag %s ********************"), *AbilityStatus.ToString()));
	}
}