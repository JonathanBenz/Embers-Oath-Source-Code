// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur

#pragma once
#include "CoreMinimal.h"
#include "GameplayAbilitySystem/GA_EmberAbility.h"
#include "GA_EmberHeal.generated.h"

UCLASS()
class EMBERSOATH_API UGA_EmberHeal : public UGA_EmberAbility
{
	GENERATED_BODY()
	
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;

	virtual FText GetDisplayName() override;
	virtual FText GetCostOfUsage(int32 Level) override;
	virtual FText GetOathPointCost() override;
};