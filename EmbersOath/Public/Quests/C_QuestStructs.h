// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Game/C_Enums.h"
#include "C_QuestStructs.generated.h"

// Structs and Data Assets are static and do not hold progress information, they just define the structure of quests and missions. Progress and completion tracking is handled in the QuestManagerComponent, which holds runtime state.

// Struct representing a single task within a quest - tracks progress and completion.
USTRUCT(BlueprintType)
struct FMissionTask
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName TaskID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine = true))
    FText Description;

    // Number of things required to complete this task
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 RequiredCount = 1;
};

// Struct representing a mission, which consists of multiple tasks.
USTRUCT(BlueprintType)
struct FMissionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTag MissionTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText MissionName;  // Display name for the mission - only for visual purposes as FText is localized

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText MissionDescription;  

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText MissionSummary;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FMissionTask> Tasks;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 XPReward = 0;
};

USTRUCT(BlueprintType)
struct FMissionProgressState
{
    GENERATED_BODY()

    // constructor
    FMissionProgressState()
        : State(EMissionState::Active)
        , CurrentTaskId(NAME_None)
		, CurrentWaypointIndex(0)
        , TaskProgressMap()
		, TaskCompletionMap()
    {}

    FMissionProgressState(EMissionState InState, FName InTaskId, int32 InWaypointIndex, TMap<FName, int32> InTaskProgressMap, TMap<FName, bool> InTaskCompletionMap)
        : State(InState)
        , CurrentTaskId(InTaskId)
        , CurrentWaypointIndex(InWaypointIndex)
		, TaskProgressMap(InTaskProgressMap)
		, TaskCompletionMap(InTaskCompletionMap)
    {}

    // Tracks if the mission is Active, Completed, or Locked (implied if not in map)
    UPROPERTY(BlueprintReadWrite, Category = "MissionState")
    EMissionState State = EMissionState::Active;

    // Tracks which task index the player is currently on
    UPROPERTY(BlueprintReadWrite, Category = "MissionState")
    FName CurrentTaskId = NAME_None;

    // Tracks which waypoitn index the player is currently on, only for the current task(for waypoint display)
    UPROPERTY(BlueprintReadWrite, Category = "MissionState")
    int32 CurrentWaypointIndex = 0;

    // To restore correct progress on load
    UPROPERTY()
    TMap<FName, int32> TaskProgressMap;

    UPROPERTY()
    TMap<FName, bool> TaskCompletionMap;
};