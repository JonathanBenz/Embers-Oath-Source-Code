// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quests/C_QuestDataAsset.h"
#include "Game/C_Structs.h"
#include "Game/C_Enums.h"
#include "Quests/I_QuestInterface.h"
#include "GameplayTagContainer.h"
#include "C_QuestManagerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnObjectiveSetActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionStarted, int32, MissionIdx);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionCompleted, int32, MissionIdx);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskCompleted, FName, TaskID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTaskProgressed, FName, TaskID, int32, CurrentCount, int32, RequiredCount);

UCLASS(meta=(BlueprintSpawnableComponent))
class EMBERSOATH_API UC_QuestManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public: //Variables
    UPROPERTY(BlueprintReadOnly)
    TArray<UC_QuestDataAsset*> AcceptedQuests;

    UPROPERTY(BlueprintReadOnly)
    UC_QuestDataAsset* CurrentTrackedQuest;

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentTrackedMissionIndex = -1;

    UPROPERTY(BlueprintReadOnly)
    FGameplayTag CurrentTrackedMissionTag;

    UPROPERTY(BlueprintReadOnly)
    FName CurrentTrackedObjectiveID;

	UPROPERTY(BlueprintReadOnly)
	int32 CurrentWaypointIndex = 0; 

    // Keeps track of all the mission progress 
    // Key: MissionTag, Value: Its current State and Objective Index
    UPROPERTY(BlueprintReadOnly, Category = "Quests|State")
    TMap<FGameplayTag, FMissionProgressState> MissionStateMap;

    // Global registry mapping every known MissionTag to its parent Quest Data Asset
    UPROPERTY(BlueprintReadOnly, Category = "Quests|State")
    TMap<FGameplayTag, UC_QuestDataAsset*> MissionToQuestMap;

    UPROPERTY()
    TMap<FGameplayTag, int32> MainQuestMissionIndexLookup;

    UPROPERTY()
    TMap<FGameplayTag, int32> SideQuestMissionIndexLookup;

public: //Functions

    UFUNCTION(BlueprintCallable, BlueprintPure)
    FORCEINLINE bool GetIsLoadingQuestState() { return bIsLoadingQuestState; }

	UFUNCTION(BlueprintCallable, Category = "Quests")
	FORCEINLINE void SetIsLoadingQuestState(bool bIsLoading) { bIsLoadingQuestState = bIsLoading; }

	UFUNCTION(BlueprintCallable, Category = "Quests")
    FORCEINLINE bool GetIsThisMissionTracked(FGameplayTag MissionTag) { return CurrentTrackedMissionTag == MissionTag; }

   // For saving/loading quest state, we can define a struct that contains all the necessary information to reconstruct the quest state when loading. This struct can then be serialized to a file or saved in a save game object.
    UFUNCTION(BlueprintCallable, Category = "Quests")
    virtual FQuestSavedState GetQuestState(bool bIsFromRestart);

    UFUNCTION(BlueprintCallable, Category = "Quests")
    virtual void LoadQuestState(FQuestSavedState SavedState);

	UFUNCTION(BlueprintCallable, Category = "Quests")
	bool PopulateVariableWhenNewQuestAccquired(UC_QuestDataAsset* Quest);
     
    UFUNCTION(BlueprintCallable, Category = "Quests")
    bool StartQuest(UC_QuestDataAsset* Quest);

    UFUNCTION(BlueprintCallable, Category = "Quests")
    bool StartMissionByTag(FGameplayTag MissionTag);

    // Sets passed mission to the currently tracked mission 
    UFUNCTION(BlueprintCallable, Category = "Quests|Tracking")
    bool SetMissionToTrack(FGameplayTag MissionToSetTracked);

    UFUNCTION(BlueprintCallable, Category = "Quests")
    void SetActiveObjective(FName TaskID);

    UFUNCTION(BlueprintCallable, Category = "Quests")
    void SetActiveObjectiveForUntrackedMission(FGameplayTag MissionTag, FName TaskID);

    UFUNCTION(BlueprintCallable, Category = "Quests")
    void ProgressTask(FName TaskID, int32 Amount = 1, bool bToSetNextObjectiveActive = true);

    UFUNCTION(BlueprintCallable, Category = "Quests")
    void CompleteMission(FGameplayTag MissionTag, bool bIsTracked);

    UFUNCTION(BlueprintCallable, Category = "Quests", meta = (DisplayName = "Get Mission Tasks"))
    TArray<FMissionTask> GetMissionTasksByTag(FGameplayTag MissionTag) const;

    UFUNCTION(BlueprintCallable)
    bool CheckIfMissionIsComplete(FGameplayTag MissionTag);

    UFUNCTION(BlueprintCallable)
    FMissionTask FindTaskInCurrentTrackedMission(FName TaskID);

    UFUNCTION(BlueprintCallable, Category = "Quests")
    FMissionTask GetNextTaskInUntrackedMission(FGameplayTag MissionTag, FName CurrentTaskID);

    UFUNCTION(BlueprintCallable)
    FMissionTask FindTaskInPassedMissionTag(FGameplayTag MissionTag, FName TaskID);

	UFUNCTION(BlueprintCallable, Category = "Quests|Waypoints")
	void IncrementWaypointIdx(FGameplayTag MissionTag);

    // UI Helpers
    UFUNCTION(BlueprintCallable, Category = "Quests|UIHelper")
    int32 GetTaskCurrentCount(FName TaskID) const { return TaskProgressMap.FindRef(TaskID); }

    UFUNCTION(BlueprintCallable, Category = "Quests|UIHelper")
    bool IsTaskCompleted(FName TaskID) const { return TaskCompletionMap.FindRef(TaskID); }

    // Gets the next task in the array after the provided TaskID
    UFUNCTION(BlueprintCallable, Category = "Quests|UIHelper")
    FMissionTask GetNextTaskInTrackedMission(FName CurrentTaskID);

	// Gets the next mission in the array after the provided MissionID
    UFUNCTION(BlueprintCallable, Category = "Quests|UIHelper")
    FMissionData GetNextMissionInTrackedQuest(int32 CompletedMissionIdx);

    // Get the exact state of any mission
    UFUNCTION(BlueprintCallable, Category = "Quests|UIHelper")
    EMissionState GetMissionState(UC_QuestDataAsset* Quest, int32 MissionIndex);

	UFUNCTION(BlueprintCallable, Category = "Quests|UIHelper")
    UC_QuestDataAsset* GetQuestsByType(EQuestType Type);

    // Finds the mission that owns a given TaskID (searches only active missions)
    UFUNCTION(BlueprintCallable, Category = "Quests|UIHelper")
    bool FindMissionFromTaskID(FName TaskID, FGameplayTag& OutMissionTag, UC_QuestDataAsset*& OutQuest, int32& OutMissionIndex);

public: //Delegates
    UPROPERTY(BlueprintAssignable)
    FOnQuestUpdated OnQuestStarted;

	UPROPERTY(BlueprintAssignable)
	FOnMissionStarted OnMissionStarted;

	UPROPERTY(BlueprintAssignable)
	FOnMissionCompleted OnMissionCompleted;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnTaskCompleted OnTaskCompleted;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnTaskProgressed OnTaskProgressed;

    UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnObjectiveSetActive OnObjectiveSetActive;

private: //Variables
    // Flag to track if we are currently loading a saved quest state
    bool bIsLoadingQuestState = false;

    // Key: TaskTag, Value: Current Progress Count
    UPROPERTY()
    TMap<FName, int32> TaskProgressMap;

    // Key: TaskTag, Value: Completion Status
    UPROPERTY()
    TMap<FName, bool> TaskCompletionMap;

private: //Functions
	// Pass and return in both by reference to avoid unnecessary copying of the entire mission lookup map
    void BuildMissionLookupForQuest(UC_QuestDataAsset* Quest, TMap<FGameplayTag, int32>& OutLookup);
    const TMap<FGameplayTag, int32>& GetCorrectMissionLookupForQuest(UC_QuestDataAsset* Quest) const;

    // Resolves the parent quest and mission index for any MissionTag via MissionToQuestMap.
    bool FindMissionIdxFromTagAndGetRelatedQuest(FGameplayTag MissionTag, UC_QuestDataAsset*& OutQuest, int32& OutMissionIndex) const;
};