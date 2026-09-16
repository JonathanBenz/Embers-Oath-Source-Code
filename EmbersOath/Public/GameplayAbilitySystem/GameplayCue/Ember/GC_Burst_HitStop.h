// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur

#pragma once
#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "GC_Burst_HitStop.generated.h"

UCLASS()
class EMBERSOATH_API UGC_Burst_HitStop : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "HitStop")
	void PlayHitStop(float Duration, AActor* TargetActor);
};