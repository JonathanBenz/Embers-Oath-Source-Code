// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur, Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Game/C_Enums.h"
#include "Game/C_Structs.h"
#include "C_GameInstance.generated.h"

class UC_UserWidget;
class UC_OptionsWidgetController;
class UC_OverlayWidgetController;
class UC_InGameWidgetController;

UCLASS()
class EMBERSOATH_API UC_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// Function to handle saving data to the text file
	UFUNCTION(BlueprintCallable, Category = "Game Data")
	void SetEmberCombatStats(int LightAttackCount, int HeavyAttackCount, float TimeSpent, int TimesTransformed);

	void SetPhoenixCombatStats(int LightAttackCount, int HeavyAttackCount, float TimeHeavyAttackWasHeld, float TimeSpent, int TimesTransformed);
	void SaveCombatDataToTextFile(); // Pass the final counter value

	virtual void Init() override;

public: //Variables
	UPROPERTY(BlueprintReadWrite, Category = "GameStart")
	bool bIsNewGame = true;

	UPROPERTY(BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
	EInputTypes CurrentInputType = EInputTypes::Keyboard;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "PauseMenu")
	bool bIsGamePaused = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RestartFromCheckpoint")
	bool bIsRestarting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "RestartFromCheckpoint")
	FName NameOfCurrentLevelToRestart;

	//Widgets and Controllers
	#pragma region Overlay Variables
		UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UC_UserWidget> OverlayWidgetClass;

		UPROPERTY(EditAnywhere)
		TSubclassOf<UC_OverlayWidgetController> OverlayWidgetControllerClass;
	#pragma endregion

	#pragma region Pasue and Options Menu Variables
		UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UC_UserWidget> PauseMenuWidgetClass;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UC_UserWidget> OptionsMenuWidgetClass;

		UPROPERTY(EditAnywhere)
		TSubclassOf<UC_OptionsWidgetController> OptionsMenuWidgetControllerClass;
	#pragma endregion

	#pragma region InGameMenu Variables
		UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<UC_UserWidget> InGameMenuWidgetClass;

		UPROPERTY(EditAnywhere)
		TSubclassOf<UC_InGameWidgetController> InGameMenuWidgetControllerClass;
	#pragma endregion

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CheckpointSystem")
	FQuestSavedState StoredQuestState = FQuestSavedState();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CheckpointSystem")
	FPlayerGameAbilitiesSavedState StoredPlayerGameAbilitiesState = FPlayerGameAbilitiesSavedState();

protected:
	virtual void Shutdown() override;

private:
	int EmberLightAttackCount = 0, EmberHeavyAttackCount = 0, TimesTransformedToPhoenix = 0;
	int PhoenixLightAttackCount = 0, PhoenixHeavyAttackCount = 0, TimesTransformedToEmber = 0;
	float TimeSpentAsEmber = 0.f, TimeSpentAsPhoenix = 0.f, TimePhoenixHeavyAttackWasHeld = 0;
	// File path to save the data
	FString SaveDirectory = FPaths::ProjectSavedDir() + TEXT("CombatData/");
	FString FileName = TEXT("EmberCombatStats.txt");
};