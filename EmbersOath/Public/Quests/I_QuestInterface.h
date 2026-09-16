// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Quests/C_QuestDataAsset.h"
#include "Game/C_Structs.h"
#include "GameplayTagContainer.h"
#include "I_QuestInterface.generated.h"

class UC_QuestManagerComponent;

// The Reflection Class
UINTERFACE(MinimalAPI, Blueprintable)
class UI_QuestInterface : public UInterface
{
	GENERATED_BODY()
};

// The Interface Class
class EMBERSOATH_API II_QuestInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest")
	UC_QuestManagerComponent* GetQuestManager() const;
};