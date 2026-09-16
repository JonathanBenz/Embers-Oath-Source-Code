// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#include "Quests/C_QuestManagerSubsystem.h"
#include "Quests/C_QuestManagerComponent.h"
#include "Quests/I_QuestInterface.h"
#include "Kismet/GameplayStatics.h"

void UC_QuestManagerSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);

    // Bind to the QuestManager once it becomes available
    UC_QuestManagerComponent* QuestManager = GetQuestManager();
    if (QuestManager)
    {
		// Bind to the QuestManager's events to know when to broadcast changes to the world's waypoints
        QuestManager->OnTaskCompleted.AddDynamic(this, &UC_QuestManagerSubsystem::HandleTaskCompleted);
		QuestManager->OnObjectiveSetActive.AddDynamic(this, &UC_QuestManagerSubsystem::RefreshAllObjectives);
        QuestManager->OnMissionStarted.AddDynamic(this, &UC_QuestManagerSubsystem::HandleMissionStarted);
    }
}

UC_QuestManagerComponent* UC_QuestManagerSubsystem::GetQuestManager() const
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetClass()->ImplementsInterface(UI_QuestInterface::StaticClass()))
    {
        return II_QuestInterface::Execute_GetQuestManager(PC);
    }
    return nullptr;
}

void UC_QuestManagerSubsystem::RefreshAllObjectives()
{
    UC_QuestManagerComponent* QuestManager = GetQuestManager();
    if (QuestManager)
    {
        // First deactivate all waypoints
        OnAllObjectivesDeactivated.Broadcast();

        // Broadcast the current ID so all listening waypoints toggle correctly
        UE_LOG(LogTemp, Warning, TEXT("RefreshAllObjectives called - CurrentTrackedObjectiveID: %s"),
            *GetQuestManager()->CurrentTrackedObjectiveID.ToString());
        OnObjectiveChanged.Broadcast(QuestManager->CurrentTrackedObjectiveID);
    }
}

void UC_QuestManagerSubsystem::HandleTaskCompleted(FName TaskID)
{
    // When a task is completed, the Manager updates CurrentObjectiveID.
    // We broadcast that new ID to the whole world.
    UC_QuestManagerComponent* QuestManager = GetQuestManager();
    if (QuestManager)
    {
        // Only broadcast if this task belongs to the tracked mission
            FMissionTask Task = QuestManager->FindTaskInCurrentTrackedMission(TaskID);
            if (Task.TaskID.IsNone()) return;

        OnObjectiveCompleted.Broadcast(TaskID);
    }
}

void UC_QuestManagerSubsystem::HandleMissionStarted(int32 MissionIdx)
{
    UC_QuestManagerComponent* QuestManager = GetQuestManager();
    if (!QuestManager) return;

    // Verify both the index AND that the current tracked mission tag is valid
    if (QuestManager->CurrentTrackedMissionIndex != MissionIdx) return;
    if (!QuestManager->CurrentTrackedMissionTag.IsValid()) return;

    UE_LOG(LogTemp, Warning, TEXT("HandleMissionStarted called - MissionIdx: %d"), MissionIdx);
    RefreshAllObjectives();
}