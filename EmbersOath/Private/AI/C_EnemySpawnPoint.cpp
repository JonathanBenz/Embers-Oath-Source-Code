// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "AI/C_EnemySpawnPoint.h"
#include "AI/C_HerdSpawnTrigger.h"
#include "Characters/Enemies/C_BaseEnemy.h"

AC_EnemySpawnPoint::AC_EnemySpawnPoint() { PrimaryActorTick.bCanEverTick = false; }

AC_BaseEnemy* AC_EnemySpawnPoint::SpawnSpecificEnemyType(AC_HerdManager* HerdManager, EEnemyArchetype EnemyType)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	EnemyActorToSpawn = GetWorld()->SpawnActor<AC_BaseEnemy>(*EnemyTypesMap.Find(EnemyType), GetActorLocation(),
		bRandomSpawnOrientation ? GetRandomSpawnedRotation() : GetRootComponent()->GetRelativeRotation(), SpawnInfo);
	EnemyActorToSpawn->SetHerdManagerOwner(HerdManager);
	return EnemyActorToSpawn;
}

AC_BaseEnemy* AC_EnemySpawnPoint::SpawnRandomEnemyType(AC_HerdManager* HerdManager)
{
	const int32 NumEnums = StaticEnum<EEnemyArchetype>()->NumEnums() - 2;
	const int32 RandomIndex = FMath::RandRange(0, NumEnums - 1);
	const EEnemyArchetype RandomEnemyArchetype = static_cast<EEnemyArchetype>(RandomIndex);

	return SpawnSpecificEnemyType(HerdManager, RandomEnemyArchetype);
}

EEnemyArchetype AC_EnemySpawnPoint::GetFirstElementKeyArchetypeFromEnemyTypesMap()
{
	for (const TPair<EEnemyArchetype, TSubclassOf<AC_BaseEnemy>>& Elem : EnemyTypesMap)
		return Elem.Key;

	return EEnemyArchetype::MAX;
}