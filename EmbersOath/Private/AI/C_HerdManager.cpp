// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "AI/C_HerdManager.h"
#include "AI/C_EnemySpawnPoint.h"
#include "AI/C_HerdSpawnTrigger.h"
#include "Checkpoint\C_CheckpointSubsystem.h"
#include "AI/C_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/C_BaseEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/C_BasePlayer.h"
#include "Characters/PS_PlayerState.h"
#include "Game/C_PlayerController.h"

AC_HerdManager::AC_HerdManager() { PrimaryActorTick.bCanEverTick = false; }

void AC_HerdManager::BeginPlay()
{
	Super::BeginPlay();
	if (MaxMeleeTokens == 0) UE_LOG(LogTemp, Error, TEXT("ERROR: Max Melee Combat Tokens Not Set In Herd Manager."));
	if (MaxRangedTokens == 0) UE_LOG(LogTemp, Error, TEXT("ERROR: Max Ranged Combat Tokens Not Set In Herd Manager."));
	if (SpawnTriggers.IsEmpty()) UE_LOG(LogTemp, Error, TEXT("ERROR: The %s Herd Manager Has No Spawn Trigger References!"), *GetName());
	if (DelaySpawnTriggersInOrder.IsEmpty()) 
		UE_LOG(LogTemp, Error, TEXT("ERROR: The %s Herd Manager Has An Empty DelaySpawnTriggersInOrder Array!"), *GetName());

	NumMeleeTokens = MaxMeleeTokens;
	NumRangedTokens = MaxRangedTokens;

	CalculateInitialAmountOfEnemies();
	NumEnemiesLeft = InitialAmountOfEnemies;
	ConsiderAdditionalPlacedEnemies();

	// Check if the herd has been previously defeated within the Checkpoint System. If so, destroy it immediately.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UC_CheckpointSubsystem* CheckpointSys = GI->GetSubsystem<UC_CheckpointSubsystem>())
		{
			if (CheckpointSys->IsEnemyOrHerdKilled(HerdIdentityTag))
			{
				// Destroy associated HerdSpawnTriggers
				for (AC_HerdSpawnTrigger* Trigger : SpawnTriggers)
				{
					if (!Trigger) continue;
					Trigger->Destroy();
				}

				for (AC_HerdSpawnTrigger* Trigger : DelaySpawnTriggersInOrder)
				{
					if (!Trigger) continue;
					Trigger->Destroy();
				}

				// Destroy Herd Manager self.
				Destroy();
			}
		}
	}

	GetWorldTimerManager().SetTimer(ExecuteHighestUtilityAgentTimerHandle, this, &AC_HerdManager::ExecuteHighestUtilityAgentTask, 0.25f, true);
}

void AC_HerdManager::CalculateInitialAmountOfEnemies()
{
	for (const TObjectPtr<AC_HerdSpawnTrigger>& Trigger : DelaySpawnTriggersInOrder)
		InitialAmountOfEnemies += Trigger->CalculateTotalEnemyCount();

	InitialAmountOfEnemies += AdditionalPlacedEnemiesToConsider.Num();
}

// Additional placed enemies are enemies that have already been preloaded into the world and assigned to a herd manager. E.g., the mech runners.
void AC_HerdManager::ConsiderAdditionalPlacedEnemies()
{
	if (AdditionalPlacedEnemiesToConsider.IsEmpty()) return;

	for (AC_BaseEnemy* PlacedEnemy : AdditionalPlacedEnemiesToConsider)
	{
		PlacedEnemy->SetHerdManagerOwner(this);
		CurrentEnemies.Add(PlacedEnemy);
		IncrementTotalXPReward(PlacedEnemy->GetXPReward());
	}
}

/// <summary>
/// When an enemy dies, this method should get called in the enemy's death method in order to let the HerdManager know to decrement the number of enemies left. 
/// </summary>
void AC_HerdManager::DecrementEnemiesLeft(AC_BaseEnemy* KilledEnemy)
{
	NumEnemiesLeft--;
	CurrentEnemies.RemoveSwap(KilledEnemy);
	if (NumEnemiesLeft <= 0) { AllEnemiesDead(); return; }

	// Check if should spawn the next wave
	if (CurrentEnemies.Num() <= EnemiesRemainingThreshold && 
		CurrentTriggerIdx < DelaySpawnTriggersInOrderArraySize)
	{
		OnWaveDefeated.Broadcast(CurrentTriggerIdx, TaskID);
		AC_HerdSpawnTrigger* NextDelayTrigger = DelaySpawnTriggersInOrder[CurrentTriggerIdx++];
		TriggerSpecificSpawnTrigger(NextDelayTrigger, true);
	}
}

/// <summary>
/// When the player first triggers a Spawn Trigger, it will remove that first instance of the Spawn Trigger from the DelaySpawnTriggersInOrder TArray. 
/// </summary>
/// <param name="FirstTriggeredInstance"> The first spawn trigger that the player overlaps with which starts the Delay encounter. </param>
void AC_HerdManager::RemoveFirstTriggeredInstanceFromDelaySpawnTriggerArray(AC_HerdSpawnTrigger* FirstTriggeredInstance)
{
	if (DelaySpawnTriggersInOrder.IsEmpty()) return;
	
	int32 idx = DelaySpawnTriggersInOrder.Find(FirstTriggeredInstance);
	if (idx != INDEX_NONE) 
		DelaySpawnTriggersInOrder.RemoveAt(idx, EAllowShrinking::Yes);

	DelaySpawnTriggersInOrderArraySize = DelaySpawnTriggersInOrder.Num();
}

/// <summary>
/// When an enemy tries to attack, it can request a token from the pool to see if it is allowed to attack or not. Returns true or false depending on if there are available tokens. 
/// </summary>
bool AC_HerdManager::RequestCombatToken(ETokenType TokenType)
{
	if (TokenType == ETokenType::ETT_Melee)
	{
		if (NumMeleeTokens == 0) return false;

		NumMeleeTokens--;
		return true;
	}

	if (TokenType == ETokenType::ETT_Ranged)
	{
		if (NumRangedTokens == 0) return false;

		NumRangedTokens--;
		return true;
	}

	return false;
}

/// <summary>
/// When an enemy's attack is either completed or interuppted, this method should be called to return their combat token back to the pool. 
/// </summary>
void AC_HerdManager::ReturnCombatToken(ETokenType TokenType)
{
	if (TokenType == ETokenType::ETT_Melee && NumMeleeTokens < MaxMeleeTokens)
		NumMeleeTokens++;
	else if (TokenType == ETokenType::ETT_Ranged && NumRangedTokens < MaxRangedTokens)
		NumRangedTokens++;
}

/// <summary>
/// When one agent switches to the combat state, this will request the rest of the herd to also switch into combat. 
/// </summary>
void AC_HerdManager::RequestAllOtherAgentsInAttackState(AC_BaseEnemy* CallingEnemy)
{
	if (CurrentEnemies.Num() <= 1) return;
	for (const TObjectPtr<AC_BaseEnemy>& Enemy : CurrentEnemies)
	{
		if (Enemy == CallingEnemy) continue;
		Enemy->GetEnemyAIController()->ImmediatelySetStateToAttack(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	}
}

/// <summary>
/// For manually spawning a specific trigger. The Delay Spawner type makes use of this. 
/// </summary>
/// <param name="SpawnTrigger"> The next spawn to trigger. </param>
/// <param name="bEnemiesStartInAttackState"> Should the enemies immediately be aware of the player upon spawning? </param>
void AC_HerdManager::TriggerSpecificSpawnTrigger(AC_HerdSpawnTrigger* Trigger, bool bEnemiesStartInAttackState)
{
	Trigger->SpawnEnemies(bEnemiesStartInAttackState);
}

/// <summary>
/// Sets bSpawnHasBeenTriggered back to false for all Herd Spawn Triggers associated with this Herd Manager. 
/// </summary>
void AC_HerdManager::ResetAllSpawnTriggers()
{
	bHerdDefeated = false;
	SetAnySpawnHasTriggered(false);
	for (const TObjectPtr<AC_HerdSpawnTrigger>& Trigger : SpawnTriggers)
		Trigger->ResetSpawnTrigger();
}

/// <summary>
/// This function gets called if any Spawn Trigger is triggered. If SpawnerType is set to Instant, then all other spawn triggers will also trigger as well. 
/// </summary>
void AC_HerdManager::SetAnySpawnHasTriggered(bool Value) { bAnySpawnHasTriggered = Value; }

/// <summary>
/// Grants XP to the player, takes them out of the combat state. Mark this herd as defeated for the checkpoint system.
/// </summary>
void AC_HerdManager::AllEnemiesDead() 
{ 
	// Handle player stuff (combat state and XP reward)
	if (APS_PlayerState* Player = Cast<APS_PlayerState>(UGameplayStatics::GetPlayerState(this, 0)))
		Player->ExitCombatState();

	if (AC_BasePlayer* Player = Cast<AC_BasePlayer>(UGameplayStatics::GetPlayerCharacter(this, 0)))
		Player->RewardXP(TotalXPReward);

	if (AC_PlayerController* PController = Cast<AC_PlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		PController->StopCombatMusic();

	OnAllEnemiesDead.Broadcast();
	bHerdDefeated = true; 

	// Mark as killed in checkpoint subsystem for respawning purposes.
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UC_CheckpointSubsystem* CheckpointSys = GI->GetSubsystem<UC_CheckpointSubsystem>())
			CheckpointSys->MarkEnemyOrHerdAsKilled(HerdIdentityTag);
	}
}

/// <summary>
/// Iterate through all enemies' Blackboard, keeping track of which enemy has the highest LocalBestUtilityScore.
/// When the enemy with the highest LocalBestUtilityScore is determined, execute that enemy's LocalBestTaskIndex. 
/// </summary>
void AC_HerdManager::ExecuteHighestUtilityAgentTask()
{
	// Disable this function if all enemies are dead. 
	if (bAnySpawnHasTriggered && CurrentEnemies.IsEmpty())
	{ 
		GetWorldTimerManager().ClearTimer(ExecuteHighestUtilityAgentTimerHandle); 
		return; 
	}

	// Step 1: Determine which agent had the highest Utility. 
	float HighestScore = -1.0f;
	AC_BaseEnemy* BestAgent = nullptr;
	for (TObjectPtr<AC_BaseEnemy> Enemy : CurrentEnemies)
	{
		float CurrentEnemyScore = Enemy->GetEnemyBlackboard()->GetValueAsFloat(FName("LocalBestUtilityScore"));
		if (CurrentEnemyScore > HighestScore)
		{
			HighestScore = CurrentEnemyScore;
			BestAgent = Enemy;
		}
	}

	// Step 2. Execute that agent's task by setting it's bAllowedToAttack blackboard key to true, letting it bypass the CanAttack Decorator. 
	if (!BestAgent) return;

	// Each task must set this bool value back to false in the blueprint. (Refer to Utility AI Architecture document). 
	BestAgent->GetEnemyBlackboard()->SetValueAsBool(FName("bAllowedToAttack"), true);
}