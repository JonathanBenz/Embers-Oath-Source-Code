// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "C_Enums.generated.h" 

UENUM(BlueprintType) 
enum class EPlayerForms : uint8 
{
	Ember UMETA(DisplayName = "Ember"),
	Phoenix UMETA(DisplayName = "Phoenix")
};

UENUM(BlueprintType, Category = "Inputs")
enum class EInputTypes : uint8
{
	None		UMETA(DisplayName = "None"),
	Keyboard	UMETA(DisplayName = "Keyboard"),
	Controller	UMETA(DisplayName = "Controller")
};

UENUM(BlueprintType, Category = "Levels")
enum class ELevelNames : uint8
{
	MainMenu		 UMETA(DisplayName = "MainMenu"),
	HiddenSettlement UMETA(DisplayName = "HiddenSettlement")
};

UENUM(BlueprintType, Category = "UI") // Enum to identify different UI widgets easily
enum class EUIWidgetId : uint8
{
	MainMenu   UMETA(DisplayName = "MainMenu"),
	Overlay    UMETA(DisplayName = "Overlay"),
	InGameMenu UMETA(DisplayName = "InGameMenu"),
	PauseMenu  UMETA(DisplayName = "PauseMenu"),
	Options    UMETA(DisplayName = "Options")
};

UENUM(BlueprintType, Category = "QuestType") // Enum to identify different quest types
enum class EQuestType : uint8
{
	MainQuest	UMETA(DisplayName = "MainQuest"),
	SideQuest	UMETA(DisplayName = "SideQuest"),
	AllQuests	UMETA(DisplayName = "AllQuests")
};

UENUM(BlueprintType)
enum class EMissionState : uint8
{
	Locked         UMETA(DisplayName = "Locked"),
	Active         UMETA(DisplayName = "Active"),
	Completed      UMETA(DisplayName = "Completed")
};

UENUM(BlueprintType)
enum class EUIOnboardingPhases : uint8
{
	NotStarted		UMETA(DisplayName = "Not Started"),
	Map				UMETA(DisplayName = "Map"),
	CompletedMap	UMETA(DisplayName = "Completed Map"),
	Quest			UMETA(DisplayName = "Quest"),
	CompletedQuest	UMETA(DisplayName = "Completed Quest"),
	Spells			UMETA(DisplayName = "Spells"),
	Completed		UMETA(DisplayName = "Completed")
};

UENUM(BlueprintType)
enum class EPlayerOnboardingPhases : uint8
{
	NotStarted			UMETA(DisplayName = "Not Started"),
	Sprint				UMETA(DisplayName = "Sprint"),
	LightAttack			UMETA(DisplayName = "Light Attack"),
	HeavyAttack			UMETA(DisplayName = "Heavy Attack"),
	CinderPool			UMETA(DisplayName = "Cinder Pool"),
	Dodge				UMETA(DisplayName = "Dodge"),
	UsingSpells			UMETA(DisplayName = "Using Spells"),
	TargetLock			UMETA(DisplayName = "Using Target Lock"),
	Transform			UMETA(DisplayName = "Transform"),
	Pitch				UMETA(DisplayName = "Pitch"),
	Boost				UMETA(DisplayName = "Boost"),
	Shoot				UMETA(DisplayName = "Shoot"),
	PhxDodge			UMETA(DisplayName = "Phoenix Dodge"),
	StationaryMode		UMETA(DisplayName = "Stationary Mode"),
	TransformBackEmber	UMETA(DisplayName = "Transform Back to Ember"),
	Completed			UMETA(DisplayName = "Completed")
};

UENUM(BlueprintType)
enum class ECheckpoints : uint8
{
	PlayerStart UMETA(DisplayName = "PlayerStart"),
	AfterMaeveShop UMETA(DisplayName = "AfterMaeveShop"),						// Triggered when Maeve Cutscene is finished playing. 
	EmberOnboardingCompleted UMETA(DisplayName = "EmberOnboardingCompleted"),	// Triggered when Ember is onboarded to her attacks and dodging. 
	FirstCombatEncounter UMETA(DisplayName = "FirstCombatEncounterDefeated"),	// Triggered when the Farm combat encounter is defeated. 
	AfterChaseSequence UMETA(DisplayName = "AfterChaseSequence"),				// 
	SecondCombatEncounter UMETA(DisplayName = "SecondCombatEncounterDefeated"),	// Triggered when the Abandoned Outpost combat encounter is defeated. 
	ThirdCombatEncounter UMETA(DisplayName = "ThirdCombatEncounterDefeated")	// Triggered when the Raven's Ridge combat encounter is defeated. 
};