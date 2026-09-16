// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur, Ayushi Parikh

#include "UI/C_BaseWidgetController.h"
#include "GameplayAbilitySystem/Data/DA_AbilityInfo.h"
#include "Characters/PS_PlayerState.h"

void UC_BaseWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
	AbilitySystemComponent = WCParams.AbilitySystemComponent;
	AttributeSet = WCParams.AttributeSet;
	GameInstance = WCParams.GameInstance;
	GameMode = WCParams.GameMode;
	PlayerController = WCParams.PlayerController;
}

void UC_BaseWidgetController::BroadcastAbilityInfo()
{
	if (!GetEmberAbilitySystemComponent()->bStartupAbilitiesGiven) return;
	//Get information about all abilities, look up their ability info, and broadcast to widgets
	
	FForEachAbility BroadcastDelegate;
	BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
	{
		FEmberAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(EmberAbilitySystemComponent -> GetAbilityFromSpec(AbilitySpec), true);
		Info.StatusTag = EmberAbilitySystemComponent->GetStatusFromSpec(AbilitySpec);
		Info.InputTag = EmberAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
		AbilityInfoDelegate.Broadcast(Info);
	});
	
	GetEmberAbilitySystemComponent()->ForEachAbility(BroadcastDelegate);
}

APS_PlayerState* UC_BaseWidgetController::GetEmberPlayerState()
{
	if (!EmberPlayerState)
		EmberPlayerState = Cast<APS_PlayerState>(PlayerState);
	
	return EmberPlayerState;
}

UASC_AbilitySystemComponent* UC_BaseWidgetController::GetEmberAbilitySystemComponent()
{
	if (!EmberAbilitySystemComponent)
		EmberAbilitySystemComponent = Cast<UASC_AbilitySystemComponent>(AbilitySystemComponent);
	
	return EmberAbilitySystemComponent;
}

UAS_BaseAttributeSet* UC_BaseWidgetController::GetEmberAttributeSet()
{
	if (!EmberAttributeSet)
		EmberAttributeSet = Cast<UAS_BaseAttributeSet>(AttributeSet);
	
	return EmberAttributeSet;
}