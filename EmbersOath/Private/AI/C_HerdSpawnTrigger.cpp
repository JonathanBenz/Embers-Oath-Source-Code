// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "AI/C_HerdSpawnTrigger.h"
#include "AI/C_EnemySpawnPoint.h"
#include "AI/C_HerdManager.h"
#include "Components/BoxComponent.h"
#include "Algo/RandomShuffle.h"
#include "Characters/Enemies/C_BaseEnemy.h"
#include "AI/C_AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Checkpoint/C_CheckpointSubsystem.h"
#include "Characters/PS_PlayerState.h"
#include "Game/C_PlayerController.h"

AC_HerdSpawnTrigger::AC_HerdSpawnTrigger() 
{ 
	PrimaryActorTick.bCanEverTick = false; 
	SpawnTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Spawn Trigger"));
	RootComponent = SpawnTrigger;
}

void AC_HerdSpawnTrigger::BeginPlay()
{
	Super::BeginPlay();
	CheckSpecificSpawnPointsForValidity();
	SpecificEnemyCount = CalculateSpecificEnemyCount();
	GenericEnemyCount = CalculateGenericEnemyCount();
	TotalEnemyCount = CalculateTotalEnemyCount();
	checkf(HerdManagerOwner, TEXT("WHAT ARE YOU DOING!! YOU NEED TO CONNECT A HERD MANAGER OWNER REFERENCE TO %s!!"), *GetName());
	if (PatrolTargets.IsEmpty()) UE_LOG(LogTemp, Warning, TEXT("Warning: Patrol Targets Not Set In Herd Manager: %s"), *GetName());
	if (GenericSpawnPoints.IsEmpty()) UE_LOG(LogTemp, Warning, TEXT("Warning: Spawn Points Not Set In Herd Manager: %s"), *GetName());
	if (TotalEnemyCount == 0) UE_LOG(LogTemp, Error, TEXT("ERROR: %s spawned a total of ZERO enemies."), *GetName());
	SpawnTrigger->OnComponentBeginOverlap.AddDynamic(this, &AC_HerdSpawnTrigger::OnBoxBeginOverlap);
	InitChosenSpawnLocations();
}

/// <summary>
/// When the player overlaps with the HerdManager's trigger volume, all enemies will spawn on the first instance of the overlap occuring. 
/// </summary>
void AC_HerdSpawnTrigger::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HerdManagerOwner) return;

	if (!bSpawnHasBeenTriggered && OtherActor->ActorHasTag(FName("EngageableTarget")) && !HerdManagerOwner->GetAnySpawnHasTriggered())
	{
		SpawnEnemies();
		HerdManagerOwner->RemoveFirstTriggeredInstanceFromDelaySpawnTriggerArray(this);
		
		if (APS_PlayerState* Player = Cast<APS_PlayerState>(UGameplayStatics::GetPlayerState(this, 0)))
		{
			Player->EnterCombatState();
			if (AC_PlayerController* PController = Cast<AC_PlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
				PController->PlayCombatMusic();
		}
	}
}

/// <summary>
/// Returns the number of Generic Enemies (not to be confused with Specific Enemies). 
/// </summary>
int32 AC_HerdSpawnTrigger::CalculateGenericEnemyCount()
{
	if (bRandomEnemySpawning) return TotalRandomEnemiesCount;

	int32 EnemyCount = 0;
	for (const TPair<EEnemyArchetype, int32>& KeyValue : GenericEnemiesToSpawn)
		EnemyCount += KeyValue.Value;
	return EnemyCount;
}

/// <summary>
/// Returns the number of Generic Enemies + Specific Enemies combined. 
/// </summary>
int32 AC_HerdSpawnTrigger::CalculateTotalEnemyCount()
{
	return CalculateSpecificEnemyCount() + CalculateGenericEnemyCount();
}

/// <summary>
/// Prebakes random spawn locations that will be used for generic enemy spawning. Accounts for both Sparse and Packed scenarios, which depends on the ratio of enemies to spawnpoints. 
/// </summary>
void AC_HerdSpawnTrigger::InitChosenSpawnLocations()
{
	// Check if generic spawning is being used. 
	if (GenericSpawnPoints.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Warning: Enemy spawn trigger has no references to generic spawn points. If using only specific spawn points, disregard this message :).")); 
		return;
	}

	ChosenSpawnPoints = GenericSpawnPoints;
	Algo::RandomShuffle(ChosenSpawnPoints);

	// If there are less enemies than there are spawn points (Sparse)
	if (GenericEnemyCount < GenericSpawnPoints.Num())
		ChosenSpawnPoints.SetNum(GenericEnemyCount); // Trim the array to the amount of enemies there are

	// If there are more enemies than there are spawn points (Packed)
	// This case will require some spawn points to spawn multiple actors, so ChosenSpawnPoints.Num() > AvailableSpawnPoints.Num()
	else if (GenericEnemyCount >= GenericSpawnPoints.Num())
	{
		const int32 Delta = GenericEnemyCount - GenericSpawnPoints.Num();
		for (int32 i = 0; i < Delta; i++)
		{
			const int32 RandIdx = FMath::RandHelper(GenericSpawnPoints.Num());
			ChosenSpawnPoints.Add(GenericSpawnPoints[RandIdx]);
		}
	}
}

/// <summary>
/// Logic for spawning enemies on the prebaked generic chosen spawn points plus any additional specific spawn points. 
/// </summary>
void AC_HerdSpawnTrigger::SpawnEnemies(bool bAllEnemiesSpawnInAttackState)
{
	bSpawnHasBeenTriggered = true;
	HerdManagerOwner->SetAnySpawnHasTriggered(true);

	if (ChosenSpawnPoints.Num() != GenericEnemyCount) UE_LOG(LogTemp, Error, TEXT("Error: ChosenSpawnPoints.Num() has a different value than GenericEnemyCount."));

	// Only execute this local code block and return early if we are randomly spawning generic enemies (or random generic enemies plus additional deterministic specific enemy spawns). 
	if (bRandomEnemySpawning)
	{
		for (AC_EnemySpawnPoint* SpawnPoint : ChosenSpawnPoints)
			SpawnRandomEnemy(SpawnPoint, bAllEnemiesSpawnInAttackState);

		for (AC_EnemySpawnPoint* SpawnPoint : SpecificSpawnPoints)
			SpawnSpecificEnemy(SpawnPoint->GetFirstElementKeyArchetypeFromEnemyTypesMap(), SpawnPoint, bAllEnemiesSpawnInAttackState);
		return;
	}

	// Get each enemy archetype to spawn within a queue.
	TQueue<EEnemyArchetype> EnemySpawningQueue;
	int32 EnemySpawningQueueSize = 0;
	for (const TPair<EEnemyArchetype, int32>& KeyValue : GenericEnemiesToSpawn)
	{
		for (int32 i = 0; i < KeyValue.Value; i++)
		{
			EnemySpawningQueue.Enqueue(KeyValue.Key);
			EnemySpawningQueueSize++;
		}
	}
	if (EnemySpawningQueueSize != GenericEnemyCount) UE_LOG(LogTemp, Error, TEXT("Error: EnemySpawningQueue did not obtain all enemies to spawn."));
	
	// Spawn all generic enemies by dequeuing our spawning queue.
	for (AC_EnemySpawnPoint* SpawnPoint : ChosenSpawnPoints)
		SpawnSpecificEnemy(EnemySpawningQueue, SpawnPoint, bAllEnemiesSpawnInAttackState);

	if (!EnemySpawningQueue.IsEmpty()) UE_LOG(LogTemp, Error, TEXT("Error: All enemies spawned, but EnemySpawningQueue is not empty."));

	// Now spawn specific enemies, if any. 
	for (AC_EnemySpawnPoint* SpawnPoint : SpecificSpawnPoints)
		SpawnSpecificEnemy(SpawnPoint->GetFirstElementKeyArchetypeFromEnemyTypesMap(), SpawnPoint, bAllEnemiesSpawnInAttackState);
}

void AC_HerdSpawnTrigger::SpawnRandomEnemy(AC_EnemySpawnPoint* SpawnPoint, bool bAllEnemiesSpawnInAttackState)
{
	AC_BaseEnemy* Enemy = SpawnPoint->SpawnRandomEnemyType(HerdManagerOwner);
	Enemy->SetPatrolTargets(PatrolTargets);
	HerdManagerOwner->CurrentEnemies.Add(Enemy);
	HerdManagerOwner->IncrementTotalXPReward(Enemy->GetXPReward());
	if (bAllEnemiesSpawnInAttackState) Enemy->GetEnemyAIController()->ImmediatelySetStateToAttack(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

/// <summary>
/// Spawn method used for spawning generic enemies from the GenericEnemiesToSpawn TMap. 
/// </summary>
void AC_HerdSpawnTrigger::SpawnSpecificEnemy(TQueue<EEnemyArchetype>& EnemySpawningQueue, AC_EnemySpawnPoint* SpawnPoint, bool bAllEnemiesSpawnInAttackState)
{
	EEnemyArchetype EnemyArchetypeToSpawn;
	EnemySpawningQueue.Dequeue(EnemyArchetypeToSpawn);
	AC_BaseEnemy* Enemy = SpawnPoint->SpawnSpecificEnemyType(HerdManagerOwner, EnemyArchetypeToSpawn);
	Enemy->SetPatrolTargets(PatrolTargets);
	HerdManagerOwner->CurrentEnemies.Add(Enemy);
	HerdManagerOwner->IncrementTotalXPReward(Enemy->GetXPReward());
	if (bAllEnemiesSpawnInAttackState) Enemy->GetEnemyAIController()->ImmediatelySetStateToAttack(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

/// <summary>
/// Spawn method used for spawning specific enemies from the SpecificSpawnPoints TArray.
/// </summary>
void AC_HerdSpawnTrigger::SpawnSpecificEnemy(EEnemyArchetype SpecificEnemy, AC_EnemySpawnPoint* SpawnPoint, bool bAllEnemiesSpawnInAttackState)
{
	AC_BaseEnemy* Enemy = SpawnPoint->SpawnSpecificEnemyType(HerdManagerOwner, SpecificEnemy);
	Enemy->SetPatrolTargets(PatrolTargets);
	HerdManagerOwner->CurrentEnemies.Add(Enemy);
	HerdManagerOwner->IncrementTotalXPReward(Enemy->GetXPReward());
	if (bAllEnemiesSpawnInAttackState) Enemy->GetEnemyAIController()->ImmediatelySetStateToAttack(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
}

/// <summary>
/// Check if the elements in TArray SpecificSpawnPoints only have EnemyTypesMap.Num() == 1. Else, it cannot be used as a specific spawn point. 
/// </summary>
void AC_HerdSpawnTrigger::CheckSpecificSpawnPointsForValidity()
{
	if (SpecificSpawnPoints.IsEmpty()) return;

	for (AC_EnemySpawnPoint* SpawnPoint : SpecificSpawnPoints)
		checkf(SpawnPoint->GetSizeOfEnemyTypesMap() == 1, TEXT("Error: Tried to use a specific spawn point that had an EnemyTypesMap of size greater than 1."));
}