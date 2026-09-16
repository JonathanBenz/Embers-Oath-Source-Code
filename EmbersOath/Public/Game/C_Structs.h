// Contributors: Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Quests/C_QuestDataAsset.h"
#include "C_Structs.generated.h"

USTRUCT(BlueprintType)
struct FWaypointDataForMap
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintReadWrite)
    FVector WaypointLocation;

    UPROPERTY(BlueprintReadWrite)
    FName AssociatedTaskID;

	// AddUnique and Contains need to know how to compare these structs, so we need to implement equality operators
    bool operator==(const FWaypointDataForMap& Other) const
    {
        return WaypointLocation == Other.WaypointLocation
            && AssociatedTaskID == Other.AssociatedTaskID;
    }
};

USTRUCT(BlueprintType)
struct FMinMaxValues
{
    GENERATED_BODY()

public:

    // A list of objectives for this level
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxValue = 0.0f;

	FMinMaxValues()
	{
	}

    FMinMaxValues(float InMinValue, float InMaxValue)
    {
        MinValue = InMinValue;
        MaxValue = InMaxValue;
    }
};


USTRUCT(BlueprintType)
struct FQuestSavedState
{
    GENERATED_BODY()

public:
    UPROPERTY()
    UC_QuestDataAsset* TrackedQuest = nullptr;

    UPROPERTY()
    int32 CurrentMissionIndex = -1;

    UPROPERTY()
    FName CurrentObjectiveID = NAME_None;

    UPROPERTY()
    FGameplayTag CurrentTrackedMissionTag;

    UPROPERTY()
    TMap<FName, int32> TaskProgressMap;

    UPROPERTY()
    TMap<FName, bool> TaskCompletionMap;


    // Save all accepted quests so we can rebuild lookups on load
    UPROPERTY()
    TArray<UC_QuestDataAsset*> AcceptedQuests;

    // Save full mission state map for background missions
    UPROPERTY()
    TMap<FGameplayTag, FMissionProgressState> MissionStateMap;

    // Constructor
    FQuestSavedState()
    {
        TrackedQuest = nullptr;
        CurrentMissionIndex = -1;
        CurrentObjectiveID = NAME_None;
		CurrentTrackedMissionTag = FGameplayTag::EmptyTag;
        TaskProgressMap.Empty();
        TaskCompletionMap.Empty();
		AcceptedQuests.Empty();
		MissionStateMap.Empty();
    }

    bool IsValid() const
    {
        return TrackedQuest != nullptr && CurrentMissionIndex >= 0;
    }
};


USTRUCT(BlueprintType)
struct FEquippedAbilitySlot
{
    GENERATED_BODY()

    UPROPERTY()
    FGameplayTag AbilityTag;

    UPROPERTY()
    FGameplayTag StatusTag;    // Unlocked or Equipped

    UPROPERTY()
    FGameplayTag SlotInputTag;

    UPROPERTY()
    int32 AbilityLevel = 1;
};


USTRUCT(BlueprintType)
struct FPlayerGameAbilitiesSavedState
{
    GENERATED_BODY()

public:

    UPROPERTY()
    bool bCanUseLightAttack = false; 
    
    UPROPERTY()
    bool bCanUseHeavyAttack = false; 

    UPROPERTY()
    bool bCanUseDodge = false;

    UPROPERTY()
    bool bCanUsePhoenixDodge = false;

    UPROPERTY()
    bool bCanUseEmberTransform = false;

    UPROPERTY()
	bool bCanUsePhoenixTransform = false;

    UPROPERTY()
	bool bCanUseShoot = false;

    UPROPERTY()
	int32 OathPoints = 0;

    UPROPERTY()
	int32 PlayerLevel = 1;

    UPROPERTY()
    bool bShownUsingSpells = false;

	UPROPERTY()
	EPlayerOnboardingPhases CurrentOnboardingPhase = EPlayerOnboardingPhases::NotStarted;

    UPROPERTY()
    EUIOnboardingPhases CurrentUIOnboardingPhase = EUIOnboardingPhases::NotStarted;

    UPROPERTY()
    TArray<FEquippedAbilitySlot> EquippedAbilitySlots;

    //Constructor
    FPlayerGameAbilitiesSavedState()
    {
        bCanUseLightAttack = false;
        bCanUseHeavyAttack = false;
        bCanUseDodge = false;
        bCanUsePhoenixDodge = false;
        bCanUseEmberTransform = false;
        bCanUsePhoenixTransform = false;
        bCanUseShoot = false;
        OathPoints = 0;
		PlayerLevel = 1;
	}
};