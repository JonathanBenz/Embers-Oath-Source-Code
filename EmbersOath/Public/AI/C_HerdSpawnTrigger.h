// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AI/C_EnemyArchetypesEnums.h"
#include "C_HerdSpawnTrigger.generated.h"

class AC_HerdManager;
class AC_EnemySpawnPoint;
class UBoxComponent;

UCLASS()
class EMBERSOATH_API AC_HerdSpawnTrigger : public AActor
{
	GENERATED_BODY()

public:
	AC_HerdSpawnTrigger();
	int32 CalculateTotalEnemyCount();
	void SpawnEnemies(bool bAllEnemiesSpawnInAttackState = false);
	FORCEINLINE void ResetSpawnTrigger() { bSpawnHasBeenTriggered = false; }
	FORCEINLINE bool HasTriggered() { return bSpawnHasBeenTriggered; }
	FORCEINLINE AC_HerdManager* GetHerdManagerOwner() { return HerdManagerOwner; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
private:
	void InitChosenSpawnLocations();
	void SpawnRandomEnemy(AC_EnemySpawnPoint* SpawnPoint, bool bAllEnemiesSpawnInAttackState);
	void SpawnSpecificEnemy(TQueue<EEnemyArchetype>& EnemySpawningQueue, AC_EnemySpawnPoint* SpawnPoint, bool bAllEnemiesSpawnInAttackState);
	void SpawnSpecificEnemy(EEnemyArchetype SpecificEnemy, AC_EnemySpawnPoint* SpawnPoint, bool bAllEnemiesSpawnInAttackState);
	void CheckSpecificSpawnPointsForValidity();
	FORCEINLINE int32 CalculateSpecificEnemyCount() { return SpecificSpawnPoints.Num(); }
	int32 CalculateGenericEnemyCount();	

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> SpawnTrigger;

	UPROPERTY(EditAnywhere, Category = "Spawn Params")
	TObjectPtr<AC_HerdManager> HerdManagerOwner;

	// Enemies to spawn if using the non-deterministic generic spawn points. 
	UPROPERTY(EditInstanceOnly, Category = "Spawn Params")
	TMap<EEnemyArchetype, int32> GenericEnemiesToSpawn; 
	
	UPROPERTY(EditInstanceOnly, Category = "Spawn Params") // Send this param to each spawned enemies
	TArray<TObjectPtr<AActor>> PatrolTargets;

	// Non-deterministic spawn points that are able to spawn any enemy archetype. 
	UPROPERTY(EditInstanceOnly, Category = "Spawn Params")
	TArray<TObjectPtr<AC_EnemySpawnPoint>> GenericSpawnPoints;

	// Deterministic spawn points using a specific enemy archetype. The EnemyTypesMap member variable of this spawn point should have a size of 1. See BP_ShooterSpawnPoint_Specific as an example. 
	UPROPERTY(EditInstanceOnly, Category = "Spawn Params")
	TArray<TObjectPtr<AC_EnemySpawnPoint>> SpecificSpawnPoints;

	UPROPERTY(EditAnywhere, Category = "Spawn Params| Random Spawning")
	bool bRandomEnemySpawning = false;

	UPROPERTY(EditAnywhere, Category = "Spawn Params| Random Spawning")
	int32 TotalRandomEnemiesCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Spawn Params")
	int32 TotalEnemyCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Spawn Params")
	int32 GenericEnemyCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Spawn Params")
	int32 SpecificEnemyCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Spawn Params")
	bool bSpawnHasBeenTriggered = false;

	UPROPERTY(VisibleAnywhere, Category = "Spawn Params")
	TArray<TObjectPtr<AC_EnemySpawnPoint>> ChosenSpawnPoints;
};