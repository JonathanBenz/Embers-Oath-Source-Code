// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "Subsystems/Subsystem.h"
#include "GameplayTagContainer.h"
#include "Quests/C_QuestStructs.h"
#include "Kismet/GameplayStatics.h"
#include "C_QuestManagerSubsystem.generated.h"

class UC_QuestManagerComponent;

//Broadcasts when the CurrentObjectiveID changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveChanged, FName, NewObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveCompleted, FName, CompletedObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllObjectivesDeactivated);

UCLASS()
// For having all the waypoints and quest-related actors listen to global quest changes without needing to bind to the QuestManagerComponent directly, we can create a QuestManagerSubsystem. This Subsystem will bind to the QuestManagerComponent's events and then broadcast its own events that any actor in the world can listen to. This way, when the QuestManagerComponent updates the current objective or completes a task, it can notify the Subsystem, which will then notify all interested parties in the world.
class EMBERSOATH_API UC_QuestManagerSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
    // Automatically call when the world is initialized
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // Returns the active QuestManagerComponent
    UFUNCTION(BlueprintCallable, Category = "Quests|Subsystem")
    UC_QuestManagerComponent* GetQuestManager() const;

    // Force all registered actors to check the current objective.
    UFUNCTION(BlueprintCallable, Category = "Quests|Subsystem")
    void RefreshAllObjectives();

    UPROPERTY(BlueprintAssignable, Category = "Quests|Subsystem")
    FOnObjectiveChanged OnObjectiveChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quests|Subsystem")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quests|Subsystem")
    FOnAllObjectivesDeactivated OnAllObjectivesDeactivated;

protected:
    //Bridge events to the Subsystem
    UFUNCTION()
    void HandleTaskCompleted(FName TaskID);

    UFUNCTION()
    void HandleMissionStarted(int32 MissionIdx);
};