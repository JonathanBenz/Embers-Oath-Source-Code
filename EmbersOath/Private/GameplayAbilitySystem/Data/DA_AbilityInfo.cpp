// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur

#include "GameplayAbilitySystem/Data/DA_AbilityInfo.h"

FEmberAbilityInfo UDA_AbilityInfo::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
	for (const FEmberAbilityInfo& Info : AbilityInformation)
	{
		if (Info.AbilityTag == AbilityTag)
		{
			return Info;
		}
	}
	
	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Cant find info for AbilityTag [%s] on AbilityInfo [%s])"), *AbilityTag.ToString(), *GetNameSafe(this));
	}
	
	return FEmberAbilityInfo();
}