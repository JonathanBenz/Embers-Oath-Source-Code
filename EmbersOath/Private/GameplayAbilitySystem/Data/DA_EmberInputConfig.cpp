// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur

#include "GameplayAbilitySystem/Data/DA_EmberInputConfig.h"

const UInputAction* UDA_EmberInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FEmberInputAction& Action : AbilityInputActions)
	{
		if (Action.InputAction && Action.InputTag == InputTag)
		{
			return Action.InputAction;
		}
	}
	
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("No InputAction found for tag: %s"), *InputTag.ToString());
	}
	
	return nullptr;
}