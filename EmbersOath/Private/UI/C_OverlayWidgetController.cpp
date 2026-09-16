// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Hritish Duvvur, Ayushi Parikh

#include "UI/C_OverlayWidgetController.h"
#include "AbilitySystemComponent.h"		
#include "GameplayAbilitySystem/AttributeSets/AS_BaseAttributeSet.h"
#include "Characters/Player/Ember/C_Ember.h"
#include "Characters/Player/Phoenix/C_Phoenix.h"
#include "Game/C_GameMode.h" 
#include "GameplayAbilitySystem/ASC_AbilitySystemComponent.h"
#include "GameplayAbilitySystem/Data/DA_AbilityInfo.h"
#include "UI/C_QuestWidgetController.h"

/// <summary>
/// Initialize initial date values for this Widget Controller. 
/// </summary>
void UC_OverlayWidgetController::BroadcastInitialValues()
{
	if (GetEmberAttributeSet())
	{
		// Now use the member variable BaseAttributeSet
		OnHealthChanged.Broadcast(GetEmberAttributeSet()->GetHealth());
		OnMaxHealthChanged.Broadcast(GetEmberAttributeSet()->GetMaxHealth());
		
		OnCinderPoolChanged.Broadcast(GetEmberAttributeSet()->GetCinderPool());
		OnMaxCinderPoolChanged.Broadcast(GetEmberAttributeSet()->GetMaxCinderPool());
	}
	
	if (GameMode->GetPhoenixPawn()) PhoenixPawn = GameMode->GetPhoenixPawn();
	if (GameMode->GetEmberPawn()) EmberPawn = GameMode->GetEmberPawn();

	GameMode->OnIsPhoenixCharActive.AddUniqueDynamic(this, &UC_OverlayWidgetController::SetShouldCrosshairBeVisible);
	GameMode->OnIsPhoenixCharActive.AddUniqueDynamic(this, &UC_OverlayWidgetController::SetShouldSpellWheelBeVisible);

	// Set the initial visibility of the crosshair based on the current player form
	GameMode->CurrentPlayerForm == EPlayerForms::Ember ? SetShouldCrosshairBeVisible(false) : SetShouldCrosshairBeVisible(true);
	// Set the initial visibility of the spell wheel based on the current player form - This is passed as false because the function itself inverts the value to set the visibility
	GameMode->CurrentPlayerForm == EPlayerForms::Ember ? SetShouldSpellWheelBeVisible(false) : SetShouldSpellWheelBeVisible(true);
}

/// <summary>
/// Have any changed data values broadcast their new values to any widgets listening for it.
/// </summary>
void UC_OverlayWidgetController::BindCallbacksToDependencies()
{
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetEmberAttributeSet()->GetHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		}
	);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetEmberAttributeSet()->GetMaxHealthAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		}
	);
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetEmberAttributeSet()->GetCinderPoolAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnCinderPoolChanged.Broadcast(Data.NewValue);
		}
	);
	
	
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(GetEmberAttributeSet()->GetMaxCinderPoolAttribute()).AddLambda(
		[this](const FOnAttributeChangeData& Data)
		{
			OnMaxCinderPoolChanged.Broadcast(Data.NewValue);
		}
	);
	
	if (GetEmberAbilitySystemComponent())
	{
		GetEmberAbilitySystemComponent()->AbilityEquipped.AddUObject(this, &UC_OverlayWidgetController::OnAbilityEquipped);

		if (GetEmberAbilitySystemComponent()->bStartupAbilitiesGiven)
			BroadcastAbilityInfo();
		else
			GetEmberAbilitySystemComponent()->AbilitiesGivenDelegate.AddUObject(this, &UC_OverlayWidgetController::BroadcastAbilityInfo);

	}
}

void UC_OverlayWidgetController::SetQuestControllerForOverlay(UC_QuestWidgetController* InQuestController)
{
	QuestController = InQuestController;

	if (QuestController)
	{
		UE_LOG(LogTemp, Warning, TEXT("OverlayWidgetController->QuestWidgetController: BroadcastInitialValues"), this);
		QuestController->BindCallbacksToDependencies();
		QuestController->BroadcastInitialValues();
	}
}

//Broadcasts to SpellGlobes that are in the Player HUD
void UC_OverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const FGameplayTag& SlotTag, const FGameplayTag& PrevSlotTag) const
{
	FEmberAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Unlocked"));
	LastSlotInfo.InputTag = PrevSlotTag;
	LastSlotInfo.AbilityTag = FGameplayTag::RequestGameplayTag(FName("EmberAbility.Spells.None"));
	
	//Broadcast Empty info if previous slot is a valid slot, otherwise it means we are equipping from an empty slot so we dont need to update the old slot
	AbilityInfoDelegate.Broadcast(LastSlotInfo);
	
	FEmberAbilityInfo NewSlotInfo = AbilityInfo -> FindAbilityInfoForTag(AbilityTag);
	NewSlotInfo.StatusTag = StatusTag;
	NewSlotInfo.InputTag = SlotTag;
	AbilityInfoDelegate.Broadcast(NewSlotInfo);
}

void UC_OverlayWidgetController::SetShouldCrosshairBeVisible(bool bCrosshairVisibility)
{
	bShouldCrosshairBeVisible = bCrosshairVisibility;
}

void UC_OverlayWidgetController::SetShouldSpellWheelBeVisible(bool bSpellWheelVisibility)
{
	bShouldSpellWheelBeVisible = !bSpellWheelVisibility;
}