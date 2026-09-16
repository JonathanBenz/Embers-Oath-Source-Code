// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#include "Quests/C_QuestManagerComponent.h"
#include "Checkpoint/C_CheckpointSubsystem.h"
#include <Quests/C_QuestManagerSubsystem.h>

bool UC_QuestManagerComponent::StartQuest(UC_QuestDataAsset* Quest)
{
	// If the same quest is being started and we are already in a mission, we shouldn't restart it. This allows for safe re-calls to StartQuest without worrying about resetting progress if the quest is already active.
    if (!Quest || AcceptedQuests.Contains(Quest)) return false;

    AcceptedQuests.Add(Quest);

    // Register all missions from this quest into the global lookup map
    for (int32 i = 0; i < Quest->Missions.Num(); ++i)
    {
        if (Quest->Missions[i].MissionTag.IsValid())
            MissionToQuestMap.Add(Quest->Missions[i].MissionTag, Quest);
    }
    
	// The passed quest becomes tracked
	if (PopulateVariableWhenNewQuestAccquired(Quest))
	{
		OnQuestStarted.Broadcast();

		//Build the mission index lookup for the new tracked quest
		switch (Quest->QuestType)
		{
		case EQuestType::MainQuest:
			BuildMissionLookupForQuest(Quest, MainQuestMissionIndexLookup);
			break;

		case EQuestType::SideQuest:
			BuildMissionLookupForQuest(Quest, SideQuestMissionIndexLookup);
			break;
		}

		// Start the first mission automatically
		if (CurrentTrackedQuest->Missions.IsValidIndex(0))
			StartMissionByTag(Quest->Missions[0].MissionTag);
	}
    
    return true;
}

bool UC_QuestManagerComponent::PopulateVariableWhenNewQuestAccquired(UC_QuestDataAsset* NewQuestToTrack)
{
    if (NewQuestToTrack && AcceptedQuests.Contains(NewQuestToTrack))
    {
        // Save previous tracked mission state before switching
        if (CurrentTrackedMissionTag.IsValid())
        {
            if (FMissionProgressState* CurrentState = MissionStateMap.Find(CurrentTrackedMissionTag))
            {
                CurrentState->TaskProgressMap = TaskProgressMap;
                CurrentState->TaskCompletionMap = TaskCompletionMap;
                CurrentState->CurrentTaskId = CurrentTrackedObjectiveID;
            }
        }

        CurrentTrackedQuest = NewQuestToTrack;
        TaskProgressMap.Empty();
        TaskCompletionMap.Empty();
        CurrentTrackedMissionIndex = -1;
        CurrentTrackedMissionTag = NewQuestToTrack->Missions[0].MissionTag;
        CurrentTrackedObjectiveID = NAME_None;

		return true;
    }

	return false;
}

bool UC_QuestManagerComponent::StartMissionByTag(FGameplayTag MissionTag)
{
   
    UC_QuestDataAsset* TempQuest = nullptr;
    int32 MissionIndex = -1;

    if (!FindMissionIdxFromTagAndGetRelatedQuest(MissionTag, TempQuest, MissionIndex))
        return false;

    // Register the mission as active in the state map regardless of tracked or background
    MissionStateMap.Add(MissionTag, FMissionProgressState(EMissionState::Active, NAME_None, 0, {}, {}));

    // Determine if this is the mission we should be tracking
    // A mission is tracked if it belongs to the current tracked quest
    bool bShouldTrack = (TempQuest == CurrentTrackedQuest) && (MissionTag == CurrentTrackedMissionTag);

    if (bShouldTrack)
    {
        CurrentTrackedQuest = TempQuest;
        CurrentTrackedMissionIndex = MissionIndex;
        CurrentTrackedObjectiveID = NAME_None;

        // Set the first objective active
        const FMissionData& Mission = CurrentTrackedQuest->Missions[CurrentTrackedMissionIndex];
        if (Mission.Tasks.IsValidIndex(0))
            SetActiveObjective(Mission.Tasks[0].TaskID);

        OnMissionStarted.Broadcast(CurrentTrackedMissionIndex);
    }

    UE_LOG(LogTemp, Warning, TEXT("StartMissionByTag: %s | TempQuest: %s | CurrentTrackedQuest: %s | bShouldTrack: %s"),
        *MissionTag.ToString(),
        TempQuest ? *TempQuest->GetName() : TEXT("None"),
        CurrentTrackedQuest ? *CurrentTrackedQuest->GetName() : TEXT("None"),
        (TempQuest == CurrentTrackedQuest) ? TEXT("TRUE") : TEXT("FALSE"));
    return true;
}

bool UC_QuestManagerComponent::SetMissionToTrack(FGameplayTag NewMissionToSetTracked)
{
    if (!NewMissionToSetTracked.IsValid()) return false;

    // Save current tracked mission state back into MissionStateMap before switching
    if (CurrentTrackedMissionTag.IsValid())
    {
        if (FMissionProgressState* CurrentState = MissionStateMap.Find(CurrentTrackedMissionTag))
        {
            CurrentState->TaskProgressMap = TaskProgressMap;
            CurrentState->TaskCompletionMap = TaskCompletionMap;
            CurrentState->CurrentTaskId = CurrentTrackedObjectiveID;
        }
    }

    // Find the parent quest this mission belongs to and the mission index
    UC_QuestDataAsset* ParentQuest = nullptr;
    int32 MissionIndex = -1;
    
    if (!FindMissionIdxFromTagAndGetRelatedQuest(NewMissionToSetTracked, ParentQuest, MissionIndex))
    {
        return false;
    }

    // Ensure the mission is actually started (Active)
    FMissionProgressState* ProgressState = MissionStateMap.Find(NewMissionToSetTracked);
    if (!ProgressState || ProgressState->State != EMissionState::Active)
    {
        // Mission is Locked (not in map) or Completed. We cannot track it.
        UE_LOG(LogTemp, Warning, TEXT("Cannot track mission. State is not Active."));
        return false;
    }

    // Update the tracking variables to the new quest and mission
    CurrentTrackedQuest = ParentQuest;
    CurrentTrackedMissionIndex = MissionIndex;
    CurrentTrackedMissionTag = NewMissionToSetTracked;


    // Resume the exact objective using our saved Task Index from the struct
    if (CurrentTrackedMissionIndex != -1 && CurrentTrackedQuest->Missions.IsValidIndex(CurrentTrackedMissionIndex))
    {
        FName SavedTaskIdx = ProgressState->CurrentTaskId;
        CurrentWaypointIndex = ProgressState->CurrentWaypointIndex;

        // If no saved task, default to the first task of the mission
        if (SavedTaskIdx.IsNone())
        {
            const FMissionData& Mission = CurrentTrackedQuest->Missions[CurrentTrackedMissionIndex];
            if (Mission.Tasks.IsValidIndex(0))
                SavedTaskIdx = Mission.Tasks[0].TaskID;
        }


        SetActiveObjective(SavedTaskIdx);
        UE_LOG(LogTemp, Warning, TEXT("SetMissionToTrack - CurrentTrackedObjectiveID after SetActiveObjective: %s"),
            *CurrentTrackedObjectiveID.ToString());
        TaskProgressMap = ProgressState->TaskProgressMap;
        TaskCompletionMap = ProgressState->TaskCompletionMap;
    }

    OnMissionStarted.Broadcast(CurrentTrackedMissionIndex);
    return true;
}

///TODO: may want to have a bool param bToHaveAnimtionDelayIfSetActiveAutomatically - This value is passed from Progress Task to check if teh objective was set active right after completing the previous one if so we need to delay the animation that shows the objective</param>
void UC_QuestManagerComponent::SetActiveObjective(FName TaskID)
{
    // Ensure we have a valid quest and an active mission to pull tasks from
    if (CurrentTrackedQuest && CurrentTrackedQuest->Missions.IsValidIndex(CurrentTrackedMissionIndex))
    {
        // Use the existing helper to verify this task exists in the current mission
        FMissionTask FoundTask = FindTaskInCurrentTrackedMission(TaskID);
        if (FoundTask.TaskID.IsNone()) return;

        // Update the task id inside the struct
        if (FMissionProgressState* MissionState = MissionStateMap.Find(CurrentTrackedQuest->Missions[CurrentTrackedMissionIndex].MissionTag))
        {
            MissionState->CurrentTaskId = TaskID;
			MissionState->CurrentWaypointIndex = 0; // Reset waypoint index to 0 whenever we set a new active objective
        }

        // Update the current objective. If TaskID was not found, this sets it to NAME_None
		CurrentTrackedObjectiveID = FoundTask.TaskID;
		// Set the current waypoint index back to 0 whenever we set a new active objective
		CurrentWaypointIndex = 0;
		if (CurrentTrackedObjectiveID != NAME_None) OnObjectiveSetActive.Broadcast();
    }
}

void UC_QuestManagerComponent::SetActiveObjectiveForUntrackedMission(FGameplayTag MissionTag, FName TaskID)
{
    UC_QuestDataAsset* ParentQuest = nullptr;
    int32 MissionIndex = -1;
    if (!FindMissionIdxFromTagAndGetRelatedQuest(MissionTag, ParentQuest, MissionIndex)) return;

    // Verify the task actually exists in this mission
    FMissionTask FoundTask = FindTaskInPassedMissionTag(MissionTag, TaskID);
    if (FoundTask.TaskID.IsNone()) return;

    // Update only the MissionProgressState — never touch CurrentTrackedObjectiveID
    if (FMissionProgressState* MissionState = MissionStateMap.Find(MissionTag))
    {
        MissionState->CurrentTaskId = TaskID;
		MissionState->CurrentWaypointIndex = 0; // Reset waypoint index to 0 whenever we set a new active objective
    }
    // No broadcast — UI is not tracking this mission
}

void UC_QuestManagerComponent::ProgressTask(FName TaskID, int32 Amount, bool bToSetNextObjectiveActive) 
{
    // Find which active mission owns this TaskID
    FGameplayTag OwningMissionTag;
    UC_QuestDataAsset* OwningQuest = nullptr;
    int32 OwningMissionIndex = -1;
    const FMissionTask* TaskDefPtr = nullptr;

    if (!FindMissionFromTaskID(TaskID, OwningMissionTag, OwningQuest, OwningMissionIndex))
        return; // TaskID not found in any active mission

    // Get the task definition
    if (OwningQuest && OwningQuest->Missions.IsValidIndex(OwningMissionIndex))
    {
        for (const FMissionTask& Task : OwningQuest->Missions[OwningMissionIndex].Tasks)
        {
            if (Task.TaskID == TaskID)
            {
                TaskDefPtr = &Task;
                break;
            }
        }
    }

    if (!TaskDefPtr) return; // TaskID not found in any active mission

    const FMissionTask& TaskDef = *TaskDefPtr;

	bool bIsTrackedMission = GetIsThisMissionTracked(OwningMissionTag);

    // Tracked mission: full behaviour, all UI delegates fire
    if (bIsTrackedMission)
    {
        if (IsTaskCompleted(TaskID)) return;

        // Get current progress or default to 0, then add the amount
        int32 NewCount = GetTaskCurrentCount(TaskID) + Amount;
        // Clamp to ensure we don't go over the required amount
        NewCount = FMath::Min(NewCount, TaskDef.RequiredCount);

        // Update map
        TaskProgressMap.Add(TaskID, NewCount);
        OnTaskProgressed.Broadcast(TaskID, NewCount, TaskDef.RequiredCount);

        // Check for task completion
        if (NewCount >= TaskDef.RequiredCount)
        {
            TaskCompletionMap.Add(TaskID, true);
            OnTaskCompleted.Broadcast(TaskID);

            // If the task is completed with this update, set the next one active
            if (bToSetNextObjectiveActive) SetActiveObjective(GetNextTaskInTrackedMission(TaskID).TaskID);

            // Sync progress back into MissionStateMap so it's always current
            if (FMissionProgressState* MissionState = MissionStateMap.Find(OwningMissionTag))
            {
                MissionState->TaskProgressMap = TaskProgressMap;
                MissionState->TaskCompletionMap = TaskCompletionMap;
            }

            // Check if the whole mission is done now
            if (CheckIfMissionIsComplete(OwningMissionTag))
                CompleteMission(OwningMissionTag, true);
        }
    }

    // Background mission: silent progress, no UI delegates
    else
    {
        FMissionProgressState* ProgressState = MissionStateMap.Find(OwningMissionTag);
        if (!ProgressState)
        {
            UE_LOG(LogTemp, Warning, TEXT("Path B: No ProgressState found for: %s"), *OwningMissionTag.ToString());
            return;
        }


        const bool* bAlreadyDone = ProgressState->TaskCompletionMap.Find(TaskID);
        if (bAlreadyDone && *bAlreadyDone)
        {
            UE_LOG(LogTemp, Warning, TEXT("Path B: Task already done: %s"), *TaskID.ToString());
            return;
        }


        int32 CurrentCount = ProgressState->TaskProgressMap.FindRef(TaskID);
        int32 NewCount = FMath::Min(CurrentCount + Amount, TaskDef.RequiredCount);

        UE_LOG(LogTemp, Warning, TEXT("Path B: TaskID: %s | CurrentCount: %d | NewCount: %d | RequiredCount: %d"),
            *TaskID.ToString(), CurrentCount, NewCount, TaskDef.RequiredCount);

        ProgressState->TaskProgressMap.Add(TaskID, NewCount);

        if (NewCount >= TaskDef.RequiredCount)
        {
            UE_LOG(LogTemp, Warning, TEXT("Path B: Task COMPLETE: %s"), *TaskID.ToString());
            ProgressState->TaskCompletionMap.Add(TaskID, true);

            FMissionTask NextTask = GetNextTaskInUntrackedMission(OwningMissionTag, TaskID);
            SetActiveObjectiveForUntrackedMission(OwningMissionTag, NextTask.TaskID);

            if (MissionStateMap.Find(OwningMissionTag))
            {
                if (CheckIfMissionIsComplete(OwningMissionTag))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Path B: Whole MissionComplete"));
                    CompleteMission(OwningMissionTag, false);
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Path B: Task not yet complete"));
        }
    }
}

void UC_QuestManagerComponent::CompleteMission(FGameplayTag MissionTag, bool bIsTracked) //Complete the mission and reward XP
{ 
    if (!MissionTag.IsValid()) return;

    UC_QuestDataAsset* TargetQuest = nullptr;
    int32 TargetMissionIndex = -1;
    if (!FindMissionIdxFromTagAndGetRelatedQuest(MissionTag, TargetQuest, TargetMissionIndex)) return;

    // Mark completed in state map regardless of tracked or background
    if (FMissionProgressState* MissionState = MissionStateMap.Find(MissionTag))
    {
        MissionState->State = EMissionState::Completed;
    }

    // TODO: Reward XP here

    if (bIsTracked)
    {
        // Find the next mission in this quest
        FMissionData NextMission = GetNextMissionInTrackedQuest(TargetMissionIndex);

        // Pre-set the tracked mission tag to the next mission BEFORE broadcasting
        // so StartMissionByTag knows to track it when it gets called
        if (NextMission.MissionTag.IsValid())
        {
            CurrentTrackedMissionTag = NextMission.MissionTag;
        }
        else
        {
            CurrentTrackedMissionTag = FGameplayTag(); // No next mission, quest is done
        }

        OnMissionCompleted.Broadcast(TargetMissionIndex);
        CurrentTrackedObjectiveID = NAME_None;
    }
    else
    {
        // Background mission: Find the next mission in this quest and auto-start it
        FMissionData NextMission = GetNextMissionInTrackedQuest(TargetMissionIndex);

        if (NextMission.MissionTag.IsValid())
        {
            // Auto-start the next mission (will remain untracked since this quest isn't tracked)
            StartMissionByTag(NextMission.MissionTag);
        }

        UE_LOG(LogTemp, Warning, TEXT("CompleteMission: Background mission completed - %s. Next mission: %s"),
            *MissionTag.ToString(),
            NextMission.MissionTag.IsValid() ? *NextMission.MissionTag.ToString() : TEXT("None"));
    }
}

bool UC_QuestManagerComponent::CheckIfMissionIsComplete(FGameplayTag MissionTag)
{
    UC_QuestDataAsset* ParentQuest = nullptr;
    int32 MissionIndex = -1;
    if (!FindMissionIdxFromTagAndGetRelatedQuest(MissionTag, ParentQuest, MissionIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("CheckIfMissionIsComplete: ResolveMission FAILED for tag: %s"), *MissionTag.ToString());
        return false;
    }

    bool bIsTrackedMission = GetIsThisMissionTracked(MissionTag);
    UE_LOG(LogTemp, Warning, TEXT("CheckIfMissionIsComplete: Tag: %s | bIsTracked: %s"), *MissionTag.ToString(), bIsTrackedMission ? TEXT("TRUE") : TEXT("FALSE"));

    const FMissionData& Mission = ParentQuest->Missions[MissionIndex];
    if (bIsTrackedMission)
    {
        // If any task is not in the lookup, or is in the lookup but false, the mission isn't done
        for (const FMissionTask& Task : Mission.Tasks)
        {
            if (!IsTaskCompleted(Task.TaskID))
            {
                UE_LOG(LogTemp, Warning, TEXT("CheckIfMissionIsComplete: Tracked task NOT complete: %s"), *Task.TaskID.ToString());
                return false;
            }
        }
    }
    else
    {
        const FMissionProgressState* ProgressState = MissionStateMap.Find(MissionTag);
        if (!ProgressState) 
        {
            UE_LOG(LogTemp, Warning, TEXT("CheckIfMissionIsComplete: No ProgressState found for tag: %s"), *MissionTag.ToString());
            return false;
        }

        for (const FMissionTask& Task : Mission.Tasks)
        {
            const bool* bDone = ProgressState->TaskCompletionMap.Find(Task.TaskID);
            UE_LOG(LogTemp, Warning, TEXT("CheckIfMissionIsComplete: Background task: %s | Done: %s"),
                *Task.TaskID.ToString(),
                (bDone && *bDone) ? TEXT("TRUE") : TEXT("FALSE"));
            if (!bDone)
                return false;
        }
    }

    // Only returns true if we successfully checked all tasks.
    UE_LOG(LogTemp, Warning, TEXT("CheckIfMissionIsComplete: RETURNING TRUE for: %s"), *MissionTag.ToString());
    return true;
}

TArray<FMissionTask> UC_QuestManagerComponent::GetMissionTasksByTag(FGameplayTag MissionTag) const 
{
    if (CurrentTrackedQuest)
    {
        if (const int32* FoundIndex = GetCorrectMissionLookupForQuest(CurrentTrackedQuest).Find(MissionTag))
        {
            if (CurrentTrackedQuest->Missions.IsValidIndex(*FoundIndex))
            {
                return CurrentTrackedQuest->Missions[*FoundIndex].Tasks;
            }
        }
    }

    return TArray<FMissionTask>();
}

FMissionTask UC_QuestManagerComponent::FindTaskInCurrentTrackedMission(FName TaskID)
{
    if (CurrentTrackedQuest && CurrentTrackedQuest->Missions.IsValidIndex(CurrentTrackedMissionIndex))
    {
        const FMissionData& CurrentMission = CurrentTrackedQuest->Missions[CurrentTrackedMissionIndex];

        for (const FMissionTask& Task : CurrentMission.Tasks)
        {
            if (Task.TaskID == TaskID)
            {
                return Task;
            }
        }
    }

    // Return an empty/invalid struct if not found
    return FMissionTask();
}

FMissionTask UC_QuestManagerComponent::FindTaskInPassedMissionTag(FGameplayTag MissionTag, FName TaskID)
{
    UC_QuestDataAsset* ParentQuest = nullptr;
    int32 MissionIndex = -1;
    
    if (!FindMissionIdxFromTagAndGetRelatedQuest(MissionTag, ParentQuest, MissionIndex))
    {
        return FMissionTask();
    }

    // Search for the task by ID
    for (const FMissionTask& Task : ParentQuest->Missions[MissionIndex].Tasks)
    {
        if (Task.TaskID == TaskID)
            return Task;
    }

    return FMissionTask();
}

void UC_QuestManagerComponent::IncrementWaypointIdx(FGameplayTag MissionTag)
{
    CurrentWaypointIndex++;
    // Update the waypoint idx inside the struct
    if (FMissionProgressState* MissionState = MissionStateMap.Find(CurrentTrackedQuest->Missions[CurrentTrackedMissionIndex].MissionTag))
    {
        MissionState->CurrentWaypointIndex = CurrentWaypointIndex;
    }
	
    if (GetWorld())
    {
        UC_QuestManagerSubsystem* Subsystem = GetWorld()->GetSubsystem<UC_QuestManagerSubsystem>();
        if (Subsystem)
        {
            Subsystem->RefreshAllObjectives();
        }
    }
}

FMissionTask UC_QuestManagerComponent::GetNextTaskInTrackedMission(FName CurrentTaskID)
{
    // ensure we have an active quest and a valid mission index
    if (CurrentTrackedQuest && CurrentTrackedQuest->Missions.IsValidIndex(CurrentTrackedMissionIndex))
    {
        const TArray<FMissionTask>& Tasks = CurrentTrackedQuest->Missions[CurrentTrackedMissionIndex].Tasks;

        for (int32 i = 0; i < Tasks.Num(); ++i)
        {
            if (Tasks[i].TaskID == CurrentTaskID)
            {
                //Found the completed task, Check if there is another task after it.
                if (Tasks.IsValidIndex(i + 1))
                {
                    return Tasks[i + 1];
                }
                else
                {
                    // This was the final task in the array, so there is no "next" task.
                    break;
                }
            }
        }
    }

    // Return an empty, invalid struct if not found or if it was the last task
    return FMissionTask();
}

FMissionTask UC_QuestManagerComponent::GetNextTaskInUntrackedMission(FGameplayTag MissionTag, FName CurrentTaskID)
{
    UC_QuestDataAsset* ParentQuest = nullptr;
    int32 MissionIndex = -1;
    if (!FindMissionIdxFromTagAndGetRelatedQuest(MissionTag, ParentQuest, MissionIndex)) return FMissionTask();

    const TArray<FMissionTask>& Tasks = ParentQuest->Missions[MissionIndex].Tasks;

    for (int32 i = 0; i < Tasks.Num(); ++i)
    {
        if (Tasks[i].TaskID == CurrentTaskID)
        {
            if (Tasks.IsValidIndex(i + 1))
                return Tasks[i + 1];
            else
                break; // Was the final task
        }
    }

    return FMissionTask();
}

FMissionData UC_QuestManagerComponent::GetNextMissionInTrackedQuest(int32 CompletedMissionIdx)
{
    // Ensure we have a valid quest asset loaded
    if (CurrentTrackedQuest)
    {
        // Check if there is a mission at the next index position
        int32 NextIndex = CompletedMissionIdx + 1;

        if (CurrentTrackedQuest->Missions.IsValidIndex(NextIndex))
        {
            // Return the mission data for the next mission
            return CurrentTrackedQuest->Missions[NextIndex];
        }
    }

    // Return an empty, invalid FMissionData struct if no next mission exists
    return FMissionData();
}

EMissionState UC_QuestManagerComponent::GetMissionState(UC_QuestDataAsset* Quest, int32 MissionIndex)
{
    if (!Quest || !Quest->Missions.IsValidIndex(MissionIndex))
    {
        return EMissionState::Locked;
    }

    FGameplayTag MissionTag = Quest->Missions[MissionIndex].MissionTag;

    // Check our MissionStateMap
    if (const FMissionProgressState* ProgressState = MissionStateMap.Find(MissionTag))
    {
        return ProgressState->State; // Will return Active or Completed
    }

    // Mission not found thus is locked
    return EMissionState::Locked;
}

FQuestSavedState UC_QuestManagerComponent::GetQuestState(bool bIsFromRestart)
{
    // Reset Everything to default values for safety
    FQuestSavedState State = FQuestSavedState();

    if (!bIsFromRestart)
    {
        State.TrackedQuest = CurrentTrackedQuest;
        State.CurrentMissionIndex = CurrentTrackedMissionIndex;
        State.CurrentObjectiveID = CurrentTrackedObjectiveID;
        State.CurrentTrackedMissionTag = CurrentTrackedMissionTag;
        State.TaskProgressMap = TaskProgressMap;
        State.TaskCompletionMap = TaskCompletionMap;
        State.AcceptedQuests = AcceptedQuests;
        State.MissionStateMap = MissionStateMap;
    }

    return State;
}

UC_QuestDataAsset* UC_QuestManagerComponent::GetQuestsByType(EQuestType Type)
{
    for (UC_QuestDataAsset* Quest : AcceptedQuests)
    {
        if (Quest && Quest->QuestType == Type)
        {
            return Quest;
        }
    }

    return nullptr;
}

bool UC_QuestManagerComponent::FindMissionFromTaskID(FName TaskID, FGameplayTag& OutMissionTag, UC_QuestDataAsset*& OutQuest, int32& OutMissionIndex)
{
    // Search through all active missions to find which one owns this TaskID
    for (auto& Pair : MissionStateMap)
    {
        if (Pair.Value.State != EMissionState::Active) continue;

        UC_QuestDataAsset* CandidateQuest = nullptr;
        int32 CandidateIndex = -1;

        if (!FindMissionIdxFromTagAndGetRelatedQuest(Pair.Key, CandidateQuest, CandidateIndex)) 
            continue;

        // Search for the task in this mission
        for (const FMissionTask& Task : CandidateQuest->Missions[CandidateIndex].Tasks)
        {
            if (Task.TaskID == TaskID)
            {
                OutMissionTag = Pair.Key;
                OutQuest = CandidateQuest;
                OutMissionIndex = CandidateIndex;
                return true;
            }
        }
    }

    // TaskID not found in any active mission
    OutMissionTag = FGameplayTag();
    OutQuest = nullptr;
    OutMissionIndex = -1;
    return false;
}

void UC_QuestManagerComponent::LoadQuestState(FQuestSavedState SavedState)
{
    if (!SavedState.IsValid()) return;

    bIsLoadingQuestState = true;

    // Restore accepted quests
    AcceptedQuests = SavedState.AcceptedQuests;

    // Rebuild MissionToQuestMap and lookup maps for all accepted quests
    MissionToQuestMap.Empty();
    MainQuestMissionIndexLookup.Empty();
    SideQuestMissionIndexLookup.Empty();

    for (UC_QuestDataAsset* Quest : AcceptedQuests)
    {
        if (!Quest) continue;

        // Register all missions into MissionToQuestMap
        for (int32 i = 0; i < Quest->Missions.Num(); ++i)
        {
            if (Quest->Missions[i].MissionTag.IsValid())
                MissionToQuestMap.Add(Quest->Missions[i].MissionTag, Quest);
        }

        // Rebuild the correct lookup map
        switch (Quest->QuestType)
        {
        case EQuestType::MainQuest:
            BuildMissionLookupForQuest(Quest, MainQuestMissionIndexLookup);
            break;
        case EQuestType::SideQuest:
            BuildMissionLookupForQuest(Quest, SideQuestMissionIndexLookup);
            break;
        default: break;
        }
    }

    // Restore mission state map (includes all background missions)
    MissionStateMap = SavedState.MissionStateMap;

    // Restore tracked mission variables
    CurrentTrackedQuest = SavedState.TrackedQuest;
    CurrentTrackedMissionIndex = SavedState.CurrentMissionIndex;
    CurrentTrackedObjectiveID = SavedState.CurrentObjectiveID;
    CurrentTrackedMissionTag = SavedState.CurrentTrackedMissionTag;
    TaskProgressMap = SavedState.TaskProgressMap;
    TaskCompletionMap = SavedState.TaskCompletionMap;

    if (CurrentTrackedQuest)
    {
        OnQuestStarted.Broadcast();

        if (CurrentTrackedMissionIndex >= 0 &&
            CurrentTrackedQuest->Missions.IsValidIndex(CurrentTrackedMissionIndex))
        {
            OnMissionStarted.Broadcast(CurrentTrackedMissionIndex);

            FGameplayTag CurrentMissionTag = CurrentTrackedQuest->Missions[CurrentTrackedMissionIndex].MissionTag;
            TArray<FMissionTask> Tasks = GetMissionTasksByTag(CurrentMissionTag);

            for (const FMissionTask& Task : Tasks)
            {
                if (IsTaskCompleted(Task.TaskID))
                {
                    OnTaskCompleted.Broadcast(Task.TaskID);
                    continue;
                }

                int32 CurrentProgress = GetTaskCurrentCount(Task.TaskID);
                if (CurrentProgress > 0)
                    OnTaskProgressed.Broadcast(Task.TaskID, CurrentProgress, Task.RequiredCount);
            }

            if (CurrentTrackedObjectiveID != NAME_None)
                OnObjectiveSetActive.Broadcast();
        }
    }

    bIsLoadingQuestState = false;
}

void UC_QuestManagerComponent::BuildMissionLookupForQuest(UC_QuestDataAsset* Quest, TMap<FGameplayTag, int32>& OutLookup)
{
    if (!Quest) return;

    for (int32 i = 0; i < Quest->Missions.Num(); ++i)
    {
        FGameplayTag Tag = Quest->Missions[i].MissionTag;
        if (Tag.IsValid())
            OutLookup.Add(Tag, i);
    }
}

const TMap<FGameplayTag, int32>& UC_QuestManagerComponent::GetCorrectMissionLookupForQuest(UC_QuestDataAsset* Quest) const
{
    if (Quest)
    {
        switch (Quest->QuestType)
        {
        case EQuestType::MainQuest: return MainQuestMissionIndexLookup;
        case EQuestType::SideQuest: return SideQuestMissionIndexLookup;
        default: break;
        }
    }

    // Safe static fallback for unhandled types
    static const TMap<FGameplayTag, int32> EmptyMap;
    return EmptyMap;
}

bool UC_QuestManagerComponent::FindMissionIdxFromTagAndGetRelatedQuest(FGameplayTag MissionTag, UC_QuestDataAsset*& OutQuest, int32& OutMissionIndex) const
{
    const UC_QuestDataAsset* const * FoundQuest = MissionToQuestMap.Find(MissionTag);
    
    if (!FoundQuest || !(*FoundQuest)) return false;

    OutQuest = const_cast<UC_QuestDataAsset*>(*FoundQuest);

    const int32* FoundIndex = GetCorrectMissionLookupForQuest(OutQuest).Find(MissionTag);
    if (!FoundIndex || !OutQuest->Missions.IsValidIndex(*FoundIndex)) return false;

    OutMissionIndex = *FoundIndex;
    return true;
}