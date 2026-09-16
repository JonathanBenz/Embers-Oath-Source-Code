// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Game/C_Enums.h"
#include "GameplayTagContainer.h"
#include "C_CheckpointSubsystem.generated.h"

UCLASS()
class EMBERSOATH_API UC_CheckpointSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Checks if the given tag is in the array of killed enemies/herds for checkpoint purposes. Called by spawn actors to determine if they should spawn or instantly die.
	UFUNCTION(BlueprintCallable, Category = "Checkpoints")
    bool IsEnemyOrHerdKilled(FGameplayTag EnemyOrHerdTagToCheck) const;

    // Called by enemies and herds when they die to mark them as killed for checkpoint purposes
	UFUNCTION(BlueprintCallable, Category = "Checkpoints")
	void MarkEnemyOrHerdAsKilled(FGameplayTag CurrentEnemyOrHerdTag);

    // Called by the spawn actors in the level during BeginPlay
    UFUNCTION(BlueprintCallable, Category = "Checkpoints")
    void RegisterCheckpointActor(ECheckpoints CheckpointName, AActor* LocationActor);

    // Stores the location based on the enum
    UFUNCTION(BlueprintCallable, Category = "Checkpoints")
    void StoreCheckpointByName(ECheckpoints CheckpointName);

    UFUNCTION(BlueprintPure, Category = "Checkpoints")
    FORCEINLINE bool IsCurrentCheckpointGreaterThan(ECheckpoints Checkpoint) { return static_cast<uint8>(LastCheckpointReached) > static_cast<uint8>(Checkpoint); }

    UFUNCTION(BlueprintPure, Category = "Checkpoints")
    FORCEINLINE bool IsCurrentCheckpointGreaterThanOrEqualTo(ECheckpoints Checkpoint) { return static_cast<uint8>(LastCheckpointReached) >= static_cast<uint8>(Checkpoint); }

public:
    UPROPERTY(BlueprintReadWrite, Category = "Checkpoints")
    FVector ActiveRestartLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Checkpoints")
    FRotator ActiveRestartRotation;

    UPROPERTY(BlueprintReadWrite, Category = "Checkpoints")
    bool bIsFirstStart = true;

    UPROPERTY(BlueprintReadWrite, Category = "Checkpoints")
	ECheckpoints LastCheckpointReached = ECheckpoints::PlayerStart;

private:
    // A map to find the spawn points we placed in the level
    UPROPERTY()
    TMap<ECheckpoints, AActor*> CheckpointRegistry;

    // List of all killed herds and individual beast tags
    FGameplayTagContainer KilledEnemyOrHerdTags;
};