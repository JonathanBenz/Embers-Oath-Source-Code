// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur

#pragma once
#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_EmberAbility.generated.h"

UCLASS()
class EMBERSOATH_API UGA_EmberAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, Category="Input")
	FGameplayTag StartupInputTag;

	//Implementation if needed in Lecture 294
	virtual FString GetDescription(int32 Level);
	virtual FString GetNextLevelDescription(int32 Level);
	static FString GetLockedDescription(int32 Level);

	virtual FText GetDisplayName();
	virtual FText GetCostOfUsage(int32 level);
	virtual FText GetOathPointCost();
};