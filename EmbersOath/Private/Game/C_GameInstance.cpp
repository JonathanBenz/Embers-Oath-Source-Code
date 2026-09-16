// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur, Ayushi Parikh

#include "Game/C_GameInstance.h"
#include "UI/C_UIManagerSubsystem.h"
#include "UI/C_OverlayWidgetController.h"
#include "UI/C_OptionsWidgetController.h"
#include "UI/C_InGameWidgetController.h"
#include "UI/C_UserWidget.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"

void UC_GameInstance::Init()
{
	Super::Init();
	
	// Register UI widget types within the UIManagerSubsystem
	UC_UIManagerSubsystem* UIManager = GetSubsystem<UC_UIManagerSubsystem>();
	if (UIManager)
	{
		UE_LOG(LogTemp, Log, TEXT("GameInstance: GEngine is valid during Init."));
		// Register overlay
		if (OverlayWidgetClass && OverlayWidgetControllerClass)
			UIManager->RegisterWidgetType(EUIWidgetId::Overlay, OverlayWidgetClass, OverlayWidgetControllerClass, 0);
		else
			UE_LOG(LogTemp, Warning, TEXT("GameInstance: Overlay widget or controller class not set - skipping registration."));

		// Register pause menu and options menu
		if (PauseMenuWidgetClass && OptionsMenuWidgetControllerClass && OptionsMenuWidgetClass)
		{
			// register the Pause Menu - base controller for both Pause and Options menus
			UIManager->RegisterWidgetType(EUIWidgetId::PauseMenu, PauseMenuWidgetClass, OptionsMenuWidgetControllerClass, 0);

			// register the options menu with pause menu's controller - shared controller
			UIManager->RegisterWidgetWithExistingController(EUIWidgetId::Options, OptionsMenuWidgetClass, EUIWidgetId::PauseMenu, 0);
		}
		else
			UE_LOG(LogTemp, Warning, TEXT("GameInstance: Pause or Options menu widget or controller class not set - skipping registration."));

		// Register in-game menu
		if (InGameMenuWidgetClass && InGameMenuWidgetControllerClass)
			UIManager->RegisterWidgetType(EUIWidgetId::InGameMenu, InGameMenuWidgetClass, InGameMenuWidgetControllerClass, 0);
		else
			UE_LOG(LogTemp, Warning, TEXT("GameInstance: InGame menu widget or controller class not set - skipping registration."));
	}
}

void UC_GameInstance::SetEmberCombatStats(int LightAttackCount, int HeavyAttackCount, float TimeSpent, int TimesTransformed)
{
	EmberLightAttackCount = LightAttackCount;
	EmberHeavyAttackCount = HeavyAttackCount;
	TimeSpentAsEmber = TimeSpent;
	TimesTransformedToPhoenix = TimesTransformed;
}

void UC_GameInstance::SetPhoenixCombatStats(int LightAttackCount, int HeavyAttackCount, float TimeHeavyAttackWasHeld, float TimeSpent, int TimesTransformed)
{
	PhoenixLightAttackCount = LightAttackCount;
	PhoenixHeavyAttackCount = HeavyAttackCount;
	TimePhoenixHeavyAttackWasHeld = TimeHeavyAttackWasHeld;
	TimeSpentAsPhoenix = TimeSpent;
	TimesTransformedToEmber = TimesTransformed;
}

void UC_GameInstance::SaveCombatDataToTextFile()
{
	int32 FileIndex = 0;
	FString BaseFileName = TEXT("CombatStats");
	FString FileExtension = TEXT(".txt");
	FString AbsoluteFilePath;

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

	do
	{
		FString CurrentFileName;
		CurrentFileName = FString::Printf(TEXT("%s_%03d%s"), *BaseFileName, FileIndex, *FileExtension);
		AbsoluteFilePath = SaveDirectory + CurrentFileName;
		FileIndex++;
	} while (PlatformFile.FileExists(*AbsoluteFilePath)); // Loop until we find a name that DOES NOT exist

	// Prepare the text content
	FString TextToSave = FString::Printf(
	TEXT("Total Ember Light Attacks: %d\nTotal Ember Heavy Attacks: %d\nTotal Time Spent as Ember: %f\nTotal Times Transformed to the Phoenix: %d\n"\
		"Total Phoenix Light Attacks: %d\nTotal Phoenix Heavy Attacks: %d\nTotal Time Heavy Attack was Held Down: %f"\
		"\nTotal Time Spent as Phoenix: %f\nTotal Times Transformed to the Ember: %d"),
	EmberLightAttackCount,                                         
	EmberHeavyAttackCount,
	TimeSpentAsEmber,
	TimesTransformedToPhoenix,
	PhoenixLightAttackCount,
	PhoenixHeavyAttackCount,
	TimePhoenixHeavyAttackWasHeld,
	TimeSpentAsPhoenix,
	TimesTransformedToEmber);                                       
	
	// Use FFileHelper to write the string to the file
	bool bSuccess = FFileHelper::SaveStringToFile(TextToSave, *AbsoluteFilePath, FFileHelper::EEncodingOptions::ForceUTF8, &IFileManager::Get());

	if (bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("Combat data saved successfully to: %s"), *AbsoluteFilePath);
	}
	else 
		UE_LOG(LogTemp, Error, TEXT("Failed to save combat data to: %s"), *AbsoluteFilePath);
}

void UC_GameInstance::Shutdown()
{
	Super::Shutdown();
	SaveCombatDataToTextFile();
}