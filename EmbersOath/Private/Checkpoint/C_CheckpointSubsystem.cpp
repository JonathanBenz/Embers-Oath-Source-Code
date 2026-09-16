// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#include "Checkpoint/C_CheckpointSubsystem.h"

bool UC_CheckpointSubsystem::IsEnemyOrHerdKilled(FGameplayTag EnemyOrHerdTagToCheck) const
{
    if (!EnemyOrHerdTagToCheck.IsValid()) return false;
    return KilledEnemyOrHerdTags.HasTagExact(EnemyOrHerdTagToCheck);
}

void UC_CheckpointSubsystem::MarkEnemyOrHerdAsKilled(FGameplayTag CurrentEnemyOrHerdTag)
{
	if (CurrentEnemyOrHerdTag.IsValid())
	{
		KilledEnemyOrHerdTags.AddTag(CurrentEnemyOrHerdTag);
		UE_LOG(LogTemp, Warning, TEXT("Marked as killed: %s"), *CurrentEnemyOrHerdTag.ToString());
	}
}

void UC_CheckpointSubsystem::RegisterCheckpointActor(ECheckpoints CheckpointName, AActor* LocationActor)
{
    if (!LocationActor) return;

    // Always add to the registry so we know where it is
    CheckpointRegistry.Add(CheckpointName, LocationActor);

    // ONLY set the active transform to PlayerStart if there isn't any saved progress.
    if (CheckpointName == ECheckpoints::PlayerStart && bIsFirstStart)
    {
        ActiveRestartLocation = LocationActor->GetActorLocation();
        ActiveRestartRotation = LocationActor->GetRootComponent()->GetRelativeRotation();
        bIsFirstStart = false;
    }
}

void UC_CheckpointSubsystem::StoreCheckpointByName(ECheckpoints CheckpointName)
{
    if (CheckpointRegistry.Contains(CheckpointName))
    {
        AActor* TargetActor = CheckpointRegistry[CheckpointName];
        if (IsValid(TargetActor))
        {
            ActiveRestartLocation = TargetActor->GetActorLocation();
            ActiveRestartRotation = TargetActor->GetRootComponent()->GetRelativeRotation();
            LastCheckpointReached = CheckpointName;

            UE_LOG(LogTemp, Log, TEXT("Checkpoint Stored: %s"), *UEnum::GetValueAsString(CheckpointName));
        }
    }
}