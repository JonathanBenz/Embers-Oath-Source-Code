// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Hritish Duvvur

#include "Characters/PS_PlayerState.h"
#include "GameplayAbilitySystem/ASC_AbilitySystemComponent.h"
#include "GameplayAbilitySystem/AttributeSets/AS_BaseAttributeSet.h"

APS_PlayerState::APS_PlayerState()
{
	// Create Ability System Component and set it to be replicated
	AbilitySystemComponent = CreateDefaultSubobject<UASC_AbilitySystemComponent>("CustomAbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	AttributeSet = CreateDefaultSubobject<UAS_BaseAttributeSet>("CustomAttributeSet");
}

UAbilitySystemComponent* APS_PlayerState::GetAbilitySystemComponent() const { return AbilitySystemComponent; }

void APS_PlayerState::AddSpellPoints(int32 AmountToBeAddedToSpellPoints)
{
	SpellPoints += AmountToBeAddedToSpellPoints;
}

void APS_PlayerState::EnterCombatState()
{
	bIsInCombat = true;
	OnCombatEntered.Broadcast();
}

void APS_PlayerState::ExitCombatState()
{
	bIsInCombat = false;
	OnCombatExited.Broadcast();
}