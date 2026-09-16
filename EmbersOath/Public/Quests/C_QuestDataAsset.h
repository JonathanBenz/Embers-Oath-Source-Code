// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Quests/C_QuestStructs.h"
#include "Game/C_Enums.h"
#include "C_QuestDataAsset.generated.h"

// Data asset representing a quest, containing multiple missions.
// Is a data asset so it can be easily created and edited in the Unreal Editor.
UCLASS(BlueprintType)
class EMBERSOATH_API UC_QuestDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    EQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    FText QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FMissionData> Missions;
};