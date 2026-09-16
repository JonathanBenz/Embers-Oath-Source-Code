// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "Game/C_CheatManager.h"
#include "Game/C_PlayerController.h"
#include "Characters/PS_PlayerState.h"
#include "Characters/C_BaseCharacter.h"
#include "Characters/C_BasePlayer.h"
#include "Characters/Player/Ember/C_Ember.h"
#include "Characters/Player/Phoenix/C_Phoenix.h"
#include "Characters/Enemies/C_BaseEnemy.h"
#include "AI/C_HerdManager.h"
#include "Kismet/GameplayStatics.h"

void UC_CheatManager::CUSTOM_CVAR_EnableInGameMenu()
{
	if (AC_PlayerController* PlayerController = Cast<AC_PlayerController>(GetPlayerController()))
		PlayerController->CurrentUIOnboardingPhase = EUIOnboardingPhases::Completed;
}

void UC_CheatManager::CUSTOM_CVAR_DisableInGameMenu()
{
	if (AC_PlayerController* PlayerController = Cast<AC_PlayerController>(GetPlayerController()))
		PlayerController->CurrentUIOnboardingPhase = EUIOnboardingPhases::NotStarted;
}

void UC_CheatManager::CUSTOM_CVAR_SkipOnboarding()
{
	if (AC_PlayerController* PlayerController = Cast<AC_PlayerController>(GetPlayerController()))
	{
		PlayerController->bSkipOnboarding = true;
		PlayerController->CurrentUIOnboardingPhase = EUIOnboardingPhases::Completed;
	}

	CUSTOM_CVAR_EnableAllAbilities();
	
	if (AC_BasePlayer* BasePlayer = Cast<AC_BasePlayer>(GetPlayerController()->GetPawn()))
		BasePlayer->PlayerLevelUp(true);
}

void UC_CheatManager::CUSTOM_CVAR_EnablePhoenixTransform()
{
	if (AC_Ember* Ember = Cast<AC_Ember>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Ember::StaticClass())))
		Ember->bCanTransform = true;
}

void UC_CheatManager::CUSTOM_CVAR_DisablePhoenixTransform()
{
	if (AC_Ember* Ember = Cast<AC_Ember>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Ember::StaticClass())))
		Ember->bCanTransform = false;
}

void UC_CheatManager::CUSTOM_CVAR_EnableEmberTransform()
{
	if (AC_Phoenix* Phoenix = Cast<AC_Phoenix>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Phoenix::StaticClass())))
		Phoenix->bCanTransform = true;
}

void UC_CheatManager::CUSTOM_CVAR_DisableEmberTransform()
{
	if (AC_Phoenix* Phoenix = Cast<AC_Phoenix>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Phoenix::StaticClass())))
		Phoenix->bCanTransform = false;
}

void UC_CheatManager::CUSTOM_CVAR_EnableLightAttack()
{
	if (AC_Ember* Ember = Cast<AC_Ember>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Ember::StaticClass())))
		Ember->bCanLightAttack = true;
	
	if (AC_Phoenix* Phoenix = Cast<AC_Phoenix>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Phoenix::StaticClass())))
		Phoenix->bCanLightAttack = true;
}

void UC_CheatManager::CUSTOM_CVAR_DisableLightAttack()
{
	if (AC_Ember* Ember = Cast<AC_Ember>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Ember::StaticClass())))
		Ember->bCanLightAttack = false;

	if (AC_Phoenix* Phoenix = Cast<AC_Phoenix>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Phoenix::StaticClass())))
		Phoenix->bCanLightAttack = false;
}

void UC_CheatManager::CUSTOM_CVAR_EnableHeavyAttack()
{
	if (AC_Ember* Ember = Cast<AC_Ember>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Ember::StaticClass())))
		Ember->bCanHeavyAttack = true;
}

void UC_CheatManager::CUSTOM_CVAR_DisableHeavyAttack()
{
	if (AC_Ember* Ember = Cast<AC_Ember>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Ember::StaticClass())))
		Ember->bCanHeavyAttack = false;
}

void UC_CheatManager::CUSTOM_CVAR_EnableDodge()
{
	if (AC_Ember* Ember = Cast<AC_Ember>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Ember::StaticClass())))
		Ember->bCanDodge = true;
}

void UC_CheatManager::CUSTOM_CVAR_DisableDodge()
{
	if (AC_Ember* Ember = Cast<AC_Ember>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Ember::StaticClass())))
		Ember->bCanDodge = false;
}

void UC_CheatManager::CUSTOM_CVAR_EnableAllAbilities()
{
	if (AC_Ember* Ember = Cast<AC_Ember>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Ember::StaticClass())))
		Ember->EnableAllAbilities();

	if (AC_Phoenix* Phoenix = Cast<AC_Phoenix>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Phoenix::StaticClass())))
	{
		Phoenix->bCanLightAttack = true;
		Phoenix->bCanTransform = true;
		Phoenix->bCanDodge = true;
	}
}

void UC_CheatManager::CUSTOM_CVAR_DisableAllAbilities()
{
	if (AC_Ember* Ember = Cast<AC_Ember>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Ember::StaticClass())))
		Ember->DisableAllAbilities();

	if (AC_Phoenix* Phoenix = Cast<AC_Phoenix>(UGameplayStatics::GetActorOfClass(GetWorld(), AC_Phoenix::StaticClass())))
	{
		Phoenix->bCanLightAttack = false;
		Phoenix->bCanTransform = false;
	}
}

void UC_CheatManager::CUSTOM_CVAR_KillYourselfNOW()
{
	if (AC_BaseCharacter* BaseCharacter = Cast<AC_BaseCharacter>(GetPlayerController()->GetPawn()))
		BaseCharacter->Die();
}

void UC_CheatManager::CUSTOM_CVAR_Godmode()
{
	if (AC_BaseCharacter* BaseCharacter = Cast<AC_BaseCharacter>(GetPlayerController()->GetPawn()))
	{
		BaseCharacter->PlayerAttributeSet->SetMaxHealth(999999.0f);
		BaseCharacter->PlayerAttributeSet->SetHealth(999999.0f);
	}
}

void UC_CheatManager::CUSTOM_CVAR_TeleportToFarm()
{
	if (AC_BaseCharacter* BaseCharacter = Cast<AC_BaseCharacter>(GetPlayerController()->GetPawn()))
		BaseCharacter->SetActorLocation(FVector(-12103.710955, -15481.648976, 3021));
}

void UC_CheatManager::CUSTOM_CVAR_TeleportToAbandonedOutpost()
{
	if (AC_BaseCharacter* BaseCharacter = Cast<AC_BaseCharacter>(GetPlayerController()->GetPawn()))
		BaseCharacter->SetActorLocation(FVector(13843.310644, 71765.112138, 2340));
}

void UC_CheatManager::CUSTOM_CVAR_TeleportToRavensRidge()
{
	if (AC_BaseCharacter* BaseCharacter = Cast<AC_BaseCharacter>(GetPlayerController()->GetPawn()))
		BaseCharacter->SetActorLocation(FVector(-8621.827827, 88574.822518, 2772.31647));
}

void UC_CheatManager::CUSTOM_CVAR_AddOathPoints(int32 OathPointsToAdd)
{
	if (APS_PlayerState* EmbersOathPlayerState = GetPlayerController()->GetPlayerState<APS_PlayerState>()) EmbersOathPlayerState->AddSpellPoints(OathPointsToAdd);
}

void UC_CheatManager::CUSTOM_CVAR_KillCurrentWave(const FString& HerdManagerName)
{
	TArray<AActor*> HerdManagers;
	TArray<TObjectPtr<AC_BaseEnemy>> EnemiesToKill;

	// Check if Farm
	if (HerdManagerName.Equals(FString("Farm"), ESearchCase::IgnoreCase))
	{
		UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AC_HerdManager::StaticClass(), FName("Farm"), HerdManagers);
		if (HerdManagers.IsEmpty()) return;

		TObjectPtr<AC_HerdManager> FarmHerdManager = Cast<AC_HerdManager>(HerdManagers[0]);
		EnemiesToKill = FarmHerdManager->CurrentEnemies;
	}

	// Check if Abandoned Outpost
	else if (HerdManagerName.Equals(FString("Abandoned Outpost"), ESearchCase::IgnoreCase) ||
			 HerdManagerName.Equals(FString("Outpost"), ESearchCase::IgnoreCase))
	{
		UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AC_HerdManager::StaticClass(), FName("AbandonedOutpost"), HerdManagers);
		if (HerdManagers.IsEmpty()) return;

		TObjectPtr<AC_HerdManager> OutpostHerdManager = Cast<AC_HerdManager>(HerdManagers[0]);
		EnemiesToKill = OutpostHerdManager->CurrentEnemies;
	}

	// Check if Raven's Ridge
	else if (HerdManagerName.Equals(FString("Raven's Ridge"), ESearchCase::IgnoreCase) ||
		HerdManagerName.Equals(FString("Ravens Ridge"), ESearchCase::IgnoreCase) ||
		HerdManagerName.Equals(FString("Ravens"), ESearchCase::IgnoreCase) ||
		HerdManagerName.Equals(FString("RR"), ESearchCase::IgnoreCase))
	{
		UGameplayStatics::GetAllActorsOfClassWithTag(GetWorld(), AC_HerdManager::StaticClass(), FName("RavensRidge"), HerdManagers);
		if (HerdManagers.IsEmpty()) return;

		TObjectPtr<AC_HerdManager> RavensRidgeHerdManager = Cast<AC_HerdManager>(HerdManagers[0]);
		EnemiesToKill = RavensRidgeHerdManager->CurrentEnemies;
	}

	for (TObjectPtr<AC_BaseEnemy> Enemy : EnemiesToKill)
		Enemy->Die();
}