// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplaytags.h"
#include "C_HerdManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllEnemiesDead);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDeathInHerd);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveDefeated, int32, WaveNumber, FName, TaskID);

class AC_BaseEnemy;
class AC_HerdSpawnTrigger;

UENUM(BlueprintType)
enum class ETokenType : uint8
{
	ETT_Melee UMETA(DisplayName = "Melee Token"),
	ETT_Ranged UMETA(DisplayName = "Ranged Token")
};

UCLASS()
class EMBERSOATH_API AC_HerdManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AC_HerdManager();
	void DecrementEnemiesLeft(AC_BaseEnemy* KilledEnemy); // Called in BaseEnemy's Death implementation
	void RemoveFirstTriggeredInstanceFromDelaySpawnTriggerArray(AC_HerdSpawnTrigger* FirstTriggeredInstance);

	UPROPERTY(BlueprintAssignable)
	FOnAllEnemiesDead OnAllEnemiesDead;

	UPROPERTY(BlueprintAssignable)
	FOnWaveDefeated OnWaveDefeated;

	UPROPERTY(BlueprintAssignable, BlueprintCallable)
	FOnEnemyDeathInHerd OnEnemyDeathInHerd;

	UFUNCTION(BlueprintCallable)
	bool RequestCombatToken(ETokenType TokenType);

	UFUNCTION(BlueprintCallable)
	void ReturnCombatToken(ETokenType TokenType);

	UFUNCTION(BlueprintCallable)
	void RequestAllOtherAgentsInAttackState(AC_BaseEnemy* CallingEnemy);

	UFUNCTION(BlueprintCallable)
	void TriggerSpecificSpawnTrigger(AC_HerdSpawnTrigger* Trigger, bool bEnemiesStartInAttackState = false);

	UFUNCTION(BlueprintCallable)
	void ResetAllSpawnTriggers();

	void SetAnySpawnHasTriggered(bool Value);

	UFUNCTION(BlueprintCallable, meta = (ExpandBoolAsExecs = "ReturnValue"))
	FORCEINLINE bool IsHerdDefeated() { return bHerdDefeated; }

	FORCEINLINE bool GetAnySpawnHasTriggered() { return bAnySpawnHasTriggered; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetEnemyCount() { return NumEnemiesLeft; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetInitialEnemyCount() { return InitialAmountOfEnemies; } // Called in C_Consideration

	FORCEINLINE void IncrementTotalXPReward(float AmountToAdd) { TotalXPReward += AmountToAdd; }
	//FVector GetHerdMidpoint(); 

public:
	// The current amount of enemies within the current wave. 
	UPROPERTY(VisibleAnywhere, Category = "Herd Manager Params")
	TArray<TObjectPtr<AC_BaseEnemy>> CurrentEnemies; // This could be useful for later, such as being used for group cohesion behaviors. 

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Herd Setup", meta = (Categories = "MechBeast"))
	FGameplayTag HerdIdentityTag;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	FName TaskID;

private:
	void AllEnemiesDead();
	void ExecuteHighestUtilityAgentTask();
	void CalculateInitialAmountOfEnemies();
	void ConsiderAdditionalPlacedEnemies();

private:
	UPROPERTY(EditAnywhere, Category = "Herd Manager Params")
	TArray<TObjectPtr<AC_HerdSpawnTrigger>> SpawnTriggers;

	// If there are enemies already placed in the level (such as the mech runners), add them here. 
	UPROPERTY(EditAnywhere, Category = "Herd Manager Params")
	TArray<TObjectPtr<AC_BaseEnemy>> AdditionalPlacedEnemiesToConsider;

	UPROPERTY(VisibleAnywhere, Category = "Herd Manager Params")
	bool bAnySpawnHasTriggered = false;

	UPROPERTY(VisibleAnywhere, Category = "Herd Manager Params")
	bool bHerdDefeated = false;

	UPROPERTY(VisibleAnywhere, Category = "Herd Manager Params")
	float TotalXPReward = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Herd Manager Params")
	int32 NumEnemiesLeft = 0;

	UPROPERTY(VisibleAnywhere, Category = "Herd Manager Params")
	int32 InitialAmountOfEnemies = 0;

	// The order of which spawn triggers should activate if Spawner type is set to Delay. Multiple instances of the same spawner are supported (e.g., if you wanted to repeat Spawner1 again). 
	UPROPERTY(EditAnywhere, Category = "Herd Manager Params| Delay Spawner")
	TArray<TObjectPtr<AC_HerdSpawnTrigger>> DelaySpawnTriggersInOrder;

	// When the value of CurrentEnemies falls below this threshold, the next SpawnTrigger from DelaySpawnTriggersInOrder will activate. It is recommended to have this value be non-zero.
	UPROPERTY(EditAnywhere, Category = "Herd Manager Params| Delay Spawner")
	int32 EnemiesRemainingThreshold = 1;

	UPROPERTY(VisibleAnywhere, Category = "Herd Manager Params| Delay Spawner")
	int32 CurrentTriggerIdx = 0;

	UPROPERTY(VisibleAnywhere, Category = "Herd Manager Params| Delay Spawner")
	int32 DelaySpawnTriggersInOrderArraySize = -1;

	UPROPERTY(EditAnywhere, Category = "Herd Manager Params| Combat Tokens")
	int32 MaxMeleeTokens;

	UPROPERTY(EditAnywhere, Category = "Herd Manager Params| Combat Tokens")
	int32 MaxRangedTokens;

	UPROPERTY(VisibleAnywhere, Category = "Herd Manager Params| Combat Tokens")
	int32 NumMeleeTokens;

	UPROPERTY(VisibleAnywhere, Category = "Herd Manager Params| Combat Tokens")
	int32 NumRangedTokens;

	FTimerHandle ExecuteHighestUtilityAgentTimerHandle;
};