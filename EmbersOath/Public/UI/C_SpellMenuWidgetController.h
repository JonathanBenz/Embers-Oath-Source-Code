// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur, Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "C_BaseWidgetController.h"
#include "GameplayTagContainer.h"
#include "C_SpellMenuWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_SevenParams(FSpellGlobeSelectedSignature, bool, bSpendPointsButtonEnabled, bool, bEquipButtonEnabled, FString, DescriptionString, FString, NextLevelDescription, FText, SpellName, FText, OathPointCost, FText, SpellCost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpellGlobeSelectedPassAssociatedAbility, const FEmberAbilityInfo&, AbilityInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitForEquipSelectionSignature, const FGameplayTag&, AbilityTypeTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpellGlobeReassignedSignature, const FGameplayTag&, AbilityTag);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFindNewSlotForEquippedSpell);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpellSelectedForFirstTimeWBP, bool, bToShow);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpellEquippedForFirstTimeWBP);

struct FSelectedAbility
{
	FGameplayTag AbilityTag = FGameplayTag();
	FGameplayTag StatusTag = FGameplayTag();
};

UCLASS(BlueprintType, Blueprintable)
class EMBERSOATH_API UC_SpellMenuWidgetController : public UC_BaseWidgetController
{
	GENERATED_BODY()
	
public: // Delegates
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature SpellPointsChanged;
	
	UPROPERTY(BlueprintAssignable)
	FSpellGlobeSelectedSignature SpellGlobeSelectedDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnSpellGlobeSelectedPassAssociatedAbility OnSpellGlobeSelectedPassAssociatedAbilityDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FWaitForEquipSelectionSignature WaitForEquipSelectionDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FWaitForEquipSelectionSignature StopWaitForEquipSelectionDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FSpellGlobeReassignedSignature SpellGlobeReassignedDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FFindNewSlotForEquippedSpell FindNewSlotForEquippedSpellDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnSpellSelectedForFirstTimeWBP OnSpellSelectedForFirstTimeWBPDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnSpellEquippedForFirstTimeWBP OnSpellEquippedForFirstTimeWBPDelegate;
	
public: //Functions
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UFUNCTION(BlueprintCallable)
	void SpellGlobeSelected(const FGameplayTag& AbilityTag);
	
	UFUNCTION(BlueprintCallable)
	void SpendPointButtonPressed();
	
	UFUNCTION(BlueprintCallable)
	void EquipButtonPressed();
	
	UFUNCTION(BlueprintCallable)
	void GlobeDeselected();
	
	UFUNCTION(BlueprintCallable)
	void SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityTypeTag);
	
	void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const FGameplayTag& SlotTag, const FGameplayTag& PrevSlotTag);
	
	UFUNCTION(BlueprintCallable)
	void ReassignSpellGlobeToNextAvailableSlot(const FEmberAbilityInfo& AbilityInfoToReassign);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 CurrentSpellPoints = 0;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag NextAvailableSlotTag = FGameplayTag();

private:
	static void ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bSpendPointsButtonEnabled, bool& bEquipButtonEnabled);
	FSelectedAbility SelectedAbility = {FGameplayTag::RequestGameplayTag(FName("EmberAbility.Spells.None")), FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Locked"))};
	bool bWaitingForEquipSelection = false;
	
	FGameplayTag SelectedSlot;
	bool bWasSpellReassigned = false;
};