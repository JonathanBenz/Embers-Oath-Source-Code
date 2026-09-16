// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur

#include "GameplayAbilitySystem/Abilities/EmberAbilities/GA_FireRain.h"

FString UGA_FireRain::GetDescription(int32 Level)
{
	return FString(TEXT("Enemies caught in the target radius of the attack receive 75 damage each."));
}

FString UGA_FireRain::GetNextLevelDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Next Level: </><Level>%d</> \n<Default>Causes much more damage. </>"), Level);
}

FText UGA_FireRain::GetDisplayName()
{
	return FText::FromString("Scorched Earth");
}

FText UGA_FireRain::GetCostOfUsage(int32 Level)
{
	return FText::FromString("2");
}

FText UGA_FireRain::GetOathPointCost()
{
	return FText::FromString("1");
}