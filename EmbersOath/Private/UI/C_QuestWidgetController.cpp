// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#include "UI/C_QuestWidgetController.h"
#include "Quests/I_QuestInterface.h"
#include "Quests/C_QuestManagerComponent.h"
#include "GameplayTagContainer.h"
#include "Game/C_PlayerController.h"

void UC_QuestWidgetController::BroadcastInitialValues()
{
    // Fix for Race condition where the widget is shown after the quest has already started, so we need to broadcast the current quest info to the WBP when the widget is shown.
    if (!PlayerController) return;

    if (!IsValid(QuestManager))
    {
        if (PlayerController->GetClass()->ImplementsInterface(UI_QuestInterface::StaticClass()))
        {
            QuestManager = II_QuestInterface::Execute_GetQuestManager(PlayerController);
        }
    }

    // If there is an active quest, broadcast its info to the WBP immediately.
    if (QuestManager)
    {
		QuestManager->SetIsLoadingQuestState(true); // Set loading state to true to prevent any progress updates from being broadcast while we are broadcasting the initial values to

        if (QuestManager->CurrentTrackedQuest)
        {
            OnQuestStartedCallback(); // This will notify the WBP to update the quest info shown in the widget when it's first shown.

            int32 CurrentMissionIdx = QuestManager->CurrentTrackedMissionIndex;

            // Ensure the index is valid before trying to access the mission data
            if (CurrentMissionIdx >= 0 && QuestManager->CurrentTrackedQuest->Missions.IsValidIndex(CurrentMissionIdx))
            {
				if (QuestManager->CurrentTrackedObjectiveID != NAME_None) OnMissionStartedCallback(CurrentMissionIdx); // Notify WBP of active mission only if there is active objective 
                
                // Get the tag for the current mission so we can look up its tasks
                FGameplayTag CurrentMissionTag = QuestManager->CurrentTrackedQuest->Missions[CurrentMissionIdx].MissionTag;

                // Loop through tasks in the current mission and notify the WBP of their progress if needed.
                TArray<FMissionTask> CurrentTasks = QuestManager->GetMissionTasksByTag(CurrentMissionTag);
                for (const FMissionTask& Task : CurrentTasks)
                {
                    // Updated to use TaskID instead of TaskTag
                    if (QuestManager->IsTaskCompleted(Task.TaskID))
                    {
                        OnTaskCompletedCallback(Task.TaskID);
                        continue; // If the task is already completed, we can skip broadcasting progress and move to the next task.
                    }

                    int32 CurrentProgress = QuestManager->GetTaskCurrentCount(Task.TaskID);

                    // Only broadcast progress if there has been some progress made on the task
                    if (CurrentProgress > 0)
                        OnTaskProgressedCallback(Task.TaskID, CurrentProgress, Task.RequiredCount);
                }

                if (QuestManager->CurrentTrackedObjectiveID != NAME_None)  OnObjectiveSetActiveCallback();
            }
        }
        
		QuestManager->SetIsLoadingQuestState(false); // Set loading state to false after we are done broadcasting the initial values
    }
}

void UC_QuestWidgetController::BindCallbacksToDependencies()
{
    if (!PlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestWidgetController[%p]: No PlayerController in BindCallbacksToDependencies"), this);
        return;
    }

    // If we already have a QuestManager and it's the same one, skip.
    // Otherwise, we MUST re-bind to the new one.
    UC_QuestManagerComponent* CurrentQM = nullptr;
    if (PlayerController->GetClass()->ImplementsInterface(UI_QuestInterface::StaticClass()))
    {
        CurrentQM = II_QuestInterface::Execute_GetQuestManager(PlayerController);
    }

    if (QuestManager == CurrentQM && bAreCallbacksBound) return;

    QuestManager = CurrentQM;
    if (QuestManager)
    {
        UE_LOG(LogTemp, Log, TEXT("QuestWidgetController[%p]: Binding to QuestManager[%p]"), this, QuestManager);

        // To prevent double binding
        QuestManager->OnQuestStarted.AddUniqueDynamic(this, &UC_QuestWidgetController::OnQuestStartedCallback);
        QuestManager->OnMissionStarted.AddUniqueDynamic(this, &UC_QuestWidgetController::OnMissionStartedCallback);
        QuestManager->OnMissionCompleted.AddUniqueDynamic(this, &UC_QuestWidgetController::OnMissionCompletedCallback);
        QuestManager->OnTaskCompleted.AddUniqueDynamic(this, &UC_QuestWidgetController::OnTaskCompletedCallback);
        QuestManager->OnTaskProgressed.AddUniqueDynamic(this, &UC_QuestWidgetController::OnTaskProgressedCallback);
		QuestManager->OnObjectiveSetActive.AddUniqueDynamic(this, &UC_QuestWidgetController::OnObjectiveSetActiveCallback);

        bAreCallbacksBound = true; // Mark as bound
    }
    else
        UE_LOG(LogTemp, Warning, TEXT("QuestWidgetController[%p]: QuestManager is null in BindCallbacksToDependencies"), this);
}


void UC_QuestWidgetController::OnQuestStartedCallback()
{
    // Broadcast to the WBP
    // This triggers the event node in your Widget Blueprint
    OnQuestStartedForWBP.Broadcast();
}

void UC_QuestWidgetController::OnMissionStartedCallback(int32 MissionIdx)
{
    UE_LOG(LogTemp, Warning, TEXT("QuestWidgetController[%p]: OnMissionStartedCallback is called"), this);
	OnMissionStartedForWBP.Broadcast(MissionIdx);
}

void UC_QuestWidgetController::OnMissionCompletedCallback(int32 MissionIdx)
{
    OnMissionCompletedForWBP.Broadcast(MissionIdx);
}

void UC_QuestWidgetController::OnTaskCompletedCallback(FName TaskID)
{
    OnTaskCompletedForWBP.Broadcast(TaskID);
}

void UC_QuestWidgetController::OnTaskProgressedCallback(FName TaskID, int32 CurrentCount, int32 RequiredCount)
{
    OnTaskProgressedForWBP.Broadcast(TaskID, CurrentCount, RequiredCount);
}

void UC_QuestWidgetController::OnObjectiveSetActiveCallback()
{
    OnObjectiveSetActiveForWBP.Broadcast();
}

void UC_QuestWidgetController::ChangeActiveQuestTypeBtn(EQuestType NewQuestType)
{
	CurrentActiveQuestTypeBtn = NewQuestType;
	OnQuestTypeBtnChanged.Broadcast();
}