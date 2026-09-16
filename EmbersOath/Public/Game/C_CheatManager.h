// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "C_CheatManager.generated.h"

UCLASS(Within = C_PlayerController)
class EMBERSOATH_API UC_CheatManager : public UCheatManager
{
	GENERATED_BODY()
	
public:
	UFUNCTION(Exec)
	void CUSTOM_CVAR_EnableInGameMenu();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_DisableInGameMenu();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_SkipOnboarding();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_EnablePhoenixTransform();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_DisablePhoenixTransform();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_EnableEmberTransform();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_DisableEmberTransform();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_EnableLightAttack();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_DisableLightAttack();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_EnableHeavyAttack();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_DisableHeavyAttack();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_EnableDodge();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_DisableDodge();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_EnableAllAbilities();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_DisableAllAbilities();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_KillYourselfNOW();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_Godmode();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_TeleportToFarm();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_TeleportToAbandonedOutpost();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_TeleportToRavensRidge();

	UFUNCTION(Exec)
	void CUSTOM_CVAR_AddOathPoints(int32 OathPointsToAdd);

	UFUNCTION(Exec)
	void CUSTOM_CVAR_KillCurrentWave(const FString& HerdManagerName);
};