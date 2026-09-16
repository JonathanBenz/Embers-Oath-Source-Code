// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
UENUM(BlueprintType)
enum class EEnemyArchetype : uint8
{
	EEA_Grunt UMETA(DisplayName = "Grunt"),
	EEA_Base UMETA(DisplayName = "Base"),
	EEA_Beefy UMETA(DisplayName = "Beefy"),
	EEA_Shooter UMETA(DisplayName = "Shooter"),
	EEA_Runner UMETA(DisplayName = "Runner"),
	EEA_PhxDodgeOnboardEnemy UMETA(DisplayName = "PhxDodge_OnboardingEnemy"),
	MAX UMETA(Hidden)
};