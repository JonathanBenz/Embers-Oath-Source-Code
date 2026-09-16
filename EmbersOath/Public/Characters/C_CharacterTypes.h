// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once

UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	ECS_NoWeaponEquipped UMETA(DisplayName = "No Weapon Equipped"),
	ECS_EquippedOneHandedWeapon UMETA(DisplayName = "One Handed Weapon Equipped"),
};

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	EES_NoState UMETA(DisplayName = "NoState"),

	EES_Dead UMETA(DisplayName = "Dead"),
	EES_Patrolling UMETA(DisplayName = "Patrolling"),
	EES_Attacking UMETA(DisplayName = "Attacking"),
	EES_Fleeing UMETA(DisplayName = "Fleeing")
};