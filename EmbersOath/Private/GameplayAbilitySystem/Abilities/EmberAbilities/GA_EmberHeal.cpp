// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur

#include "GameplayAbilitySystem/Abilities/EmberAbilities/GA_EmberHeal.h"

FString UGA_EmberHeal::GetDescription(int32 Level)
{
	return FString(TEXT("Heals for 20% of your max health."));
}

FString UGA_EmberHeal::GetNextLevelDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Next Level: </><Level>%d</> \n<Default>Causes much more damage. </>"), Level);
}

FText UGA_EmberHeal::GetDisplayName()
{
	return FText::FromString("Heal");
}

FText UGA_EmberHeal::GetCostOfUsage(int32 Level)
{
	return FText::FromString("1");
}

FText UGA_EmberHeal::GetOathPointCost()
{
	return FText::FromString("1");
}