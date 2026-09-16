// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AI/C_EnemyArchetypesEnums.h"
#include "C_EnemySpawnPoint.generated.h"

class AC_BaseEnemy;
class AC_HerdManager;

UCLASS(meta=(PrioritizeCategories = "Spawner"))
class EMBERSOATH_API AC_EnemySpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	AC_EnemySpawnPoint();

	UFUNCTION(BlueprintCallable)
	AC_BaseEnemy* SpawnSpecificEnemyType(AC_HerdManager* HerdManager, EEnemyArchetype EnemyType);

	UFUNCTION(BlueprintCallable)
	AC_BaseEnemy* SpawnRandomEnemyType(AC_HerdManager* HerdManager);

	FORCEINLINE int32 GetSizeOfEnemyTypesMap() { return EnemyTypesMap.Num(); }
	EEnemyArchetype GetFirstElementKeyArchetypeFromEnemyTypesMap();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Spawner")
	TMap<EEnemyArchetype, TSubclassOf<AC_BaseEnemy>> EnemyTypesMap;

	UPROPERTY(VisibleAnywhere, Category = "Spawner")
	TObjectPtr<AC_BaseEnemy> EnemyActorToSpawn;

	UPROPERTY(EditAnywhere, Category = "Spawner")
	bool bRandomSpawnOrientation = true;

	FORCEINLINE FRotator GetRandomSpawnedRotation() { return FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f); }
};