// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "UI/C_BaseWidgetController.h"
#include "GameplayTagContainer.h"
#include "C_QuestWidgetController.generated.h"

class UC_QuestManagerComponent;

// Delegate to notify when the quest is updated to the WBP - 
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestStartedForWBP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuestTypeBtnChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnObjectiveSetActiveForWBP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewMissionSetToTrackWBP, FGameplayTag, NewMissionSetToTrack);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionStartedForWBP, int32, MissionIdx);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMissionCompletedForWBP, int32, MissionIdx);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTaskCompletedForWBP, FName, TaskID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnTaskProgressedForWBP, FName, TaskID, int32, CurrentCount, int32, RequiredCount);

UCLASS(BlueprintType, Blueprintable)
class EMBERSOATH_API UC_QuestWidgetController : public UC_BaseWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	// Delegate to notify the WBP when the quest is updated
	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnQuestStartedForWBP OnQuestStartedForWBP;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnMissionStartedForWBP OnMissionStartedForWBP;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnMissionCompletedForWBP OnMissionCompletedForWBP;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnTaskCompletedForWBP OnTaskCompletedForWBP;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnTaskProgressedForWBP OnTaskProgressedForWBP;

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnObjectiveSetActiveForWBP OnObjectiveSetActiveForWBP;

	UPROPERTY(BlueprintAssignable, Category = "Quest UI")
	FOnQuestTypeBtnChanged OnQuestTypeBtnChanged;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Quest UI")
	FOnNewMissionSetToTrackWBP OnNewMissionSetToTrackWBP;

	UFUNCTION(BlueprintCallable, Category = "Quest UI")
	void ChangeActiveQuestTypeBtn(EQuestType NewQuestType);

public: // Variables

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	UC_QuestManagerComponent* QuestManager = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Quest")
	EQuestType CurrentActiveQuestTypeBtn = EQuestType::MainQuest; // Default to Main Quest, can be changed in the WBP when the player clicks the button to switch between quest types.

private:
	bool bAreCallbacksBound = false;

	// Callback when the quest is updated in the Quest Manager Component just to forward to the WBP
	UFUNCTION()
	void OnQuestStartedCallback();

	UFUNCTION()
	void OnMissionStartedCallback(int32 MissionIdx);

	UFUNCTION()
	void OnMissionCompletedCallback(int32 MissionIdx);

	UFUNCTION()
	void OnTaskCompletedCallback(FName TaskID);

	UFUNCTION()
	void OnTaskProgressedCallback(FName TaskID, int32 CurrentCount, int32 RequiredCount);

	UFUNCTION()
	void OnObjectiveSetActiveCallback();
};