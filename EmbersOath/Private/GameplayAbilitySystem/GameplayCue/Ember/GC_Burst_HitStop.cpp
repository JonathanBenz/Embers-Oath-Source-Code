// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur

#include "GameplayAbilitySystem/GameplayCue/Ember/GC_Burst_HitStop.h"

#include "Kismet/GameplayStatics.h"

void UGC_Burst_HitStop::PlayHitStop(float Duration, AActor* TargetActor)
{
	// 1. Slow down time globally
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.05f);

	// 2. Set a timer to bring time back to normal
	FTimerHandle HitStopHandle;
	TargetActor -> GetWorldTimerManager().SetTimer(HitStopHandle, [this]
	{
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
	}, Duration, false);
}