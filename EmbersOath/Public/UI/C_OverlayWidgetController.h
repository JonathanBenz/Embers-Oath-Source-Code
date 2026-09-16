// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Hritish Duvvur, Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySystem/ASC_AbilitySystemComponent.h"
#include "UI/C_BaseWidgetController.h"
#include "C_OverlayWidgetController.generated.h"

class UDA_AbilityInfo;
class UAS_BaseAttributeSet;
class UC_UserWidget;
class AC_Phoenix;
class AC_Ember;
class UC_QuestWidgetController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyCountInHerdChanged, FText, EnemyCounterText);

UCLASS(BlueprintType, Blueprintable)
class EMBERSOATH_API UC_OverlayWidgetController : public UC_BaseWidgetController
{
	GENERATED_BODY()
	
public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Combat|HerdManagerCallable")
	FText UpdateEnemyCount(int32 InitialEnemyCount, int32 CurrentEnemiesLeft);

public:
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedSignature OnMaxHealthChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedSignature OnCinderPoolChanged;
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnAttributeChangedSignature OnMaxCinderPoolChanged;

	// Function to assign it
	UFUNCTION(BlueprintCallable, Category="Quests")
	void SetQuestControllerForOverlay(UC_QuestWidgetController* InQuestController);

	// The existing Quest Controller that we will borrow
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UC_QuestWidgetController> QuestController;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Combat|HerdManagerCallable")
	FOnEnemyCountInHerdChanged OnEnemyCountInHerdChanged;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "UICrosshair", meta = (AllowPrivateAccess = "true"))
	bool bShouldCrosshairBeVisible;

	UPROPERTY(BlueprintReadOnly, Category = "UISpellWheel", meta = (AllowPrivateAccess = "true"))
	bool bShouldSpellWheelBeVisible;
	
	void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const FGameplayTag& SlotTag, const FGameplayTag& PrevSlotTag) const;
	
	//Function to find a Data Table and return the row
	template<typename T>
	T* GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag);

private:
	UPROPERTY()
	const UAS_BaseAttributeSet* BaseAttributeSet;
	
	UPROPERTY(BlueprintReadOnly, Category = "PlayerPawn", meta = (AllowPrivateAccess = "true"))
	AC_Ember* EmberPawn;

	UPROPERTY(BlueprintReadOnly, Category = "PlayerPawn", meta = (AllowPrivateAccess = "true"))
	AC_Phoenix* PhoenixPawn;

private:
	UFUNCTION()
	void SetShouldCrosshairBeVisible(bool bCrosshairVisibility);

	UFUNCTION()
	void SetShouldSpellWheelBeVisible(bool bSpellWheelVisibility);
};

template <typename T>
T* UC_OverlayWidgetController::GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag)
{
	return DataTable->FindRow<T>(Tag.GetTagName(), TEXT(""));
}