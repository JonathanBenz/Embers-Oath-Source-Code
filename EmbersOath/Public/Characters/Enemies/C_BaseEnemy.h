// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "Characters/C_BaseCharacter.h"
#include "Gameplaytags.h"
#include "Characters/C_CharacterTypes.h"
#include "C_BaseEnemy.generated.h"

class UBehaviorTree;
class AC_AIController;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Damage;
class UC_HealthBarComponent;
class AC_BasePlayer;
class AC_HerdManager;
class AC_Weapon;
class UBTTask_UtilityBase;
class UBlackboardComponent;

UENUM(BlueprintType)
enum class EEnemyMovementSpeed : uint8
{
	EEMS_None UMETA(DisplayName = "None"),
	EEMS_Walk UMETA(DisplayName = "Walk"),
	EEMS_Run UMETA(DisplayName = "Run"),
	EEMS_Flee UMETA(DisplayName = "Flee")
};

UCLASS()
class EMBERSOATH_API AC_BaseEnemy : public AC_BaseCharacter
{
	GENERATED_BODY()

public:
	AC_BaseEnemy();
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void Die_Implementation() override;
	bool IsTaskOnCooldown(const int32 TaskID) const;
	void PlayPassiveToAttackTransitionAnimation();

	UFUNCTION(BlueprintCallable)
	void HideHealthBar();

	UFUNCTION(BlueprintCallable)
	void ShowHealthBar();

	UFUNCTION(BlueprintCallable)
	void StartTaskCooldown(const int32 TaskID, const float CooldownTime);

	/* Getters */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE AC_BasePlayer* GetPlayer() { return Player; }
	FORCEINLINE UBehaviorTree* GetBehaviorTree() { return BehaviorTree; }
	FORCEINLINE AC_AIController* GetEnemyAIController() { return EnemyAIController; }
	FORCEINLINE UBlackboardComponent* GetEnemyBlackboard() { return Blackboard; }
	FORCEINLINE FVector GetPlayerLocation() { return PlayerLocation; }
	FORCEINLINE int32 GetTimesUsedLightAttack() { return LightAttackTracker; }
	FORCEINLINE float GetWalkSpeed() { return WalkSpeed; }
	FORCEINLINE float GetRunSpeed() { return RunSpeed; }
	FORCEINLINE float GetFleeSpeed() { return FleeSpeed; }
	FORCEINLINE float GetAttackRange() { return AttackRange; }
	FORCEINLINE float GetStrafeRange() { return StrafeRange; }
	FORCEINLINE float GetXPReward() { return XPToReward; }
	FORCEINLINE float GetDistanceToPlayer() { return DistanceToPlayer; }
	FORCEINLINE float GetMaxSpeed() { return RunSpeed; }
	FORCEINLINE float GetPoisePercentage() { return FMath::Clamp(PlayerAttributeSet->GetPoise() / PlayerAttributeSet->GetMaxPoise(), 0.0f, 1.0f); }
			    float GetCurrentAgentSpeed();
	FORCEINLINE bool PlayerIsDead() { return bPlayerIsDead; }
	FORCEINLINE bool PlayerIsGrounded() { return bPlayerIsGrounded; }
	FORCEINLINE bool PlayerIsPhoenix() { return bPlayerIsPhoenix; }
	FORCEINLINE bool IsInFrontOfPlayer() { return bIsInFrontOfPlayer; }
			    bool CanSeePlayer();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsEnemyDead() { return !IsAlive(); }

	UFUNCTION(BlueprintCallable)
	void UpdateHealthbar();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE AActor* GetCombatTarget() { return CombatTarget; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE TArray<AActor*> GetPatrolTargets() { return PatrolTargets; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE AC_HerdManager* GetHerdManager() { return HerdManagerOwner; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FVector GetPreferredStrafeDir() { return FMath::RandBool() ? -1.0f * GetActorRightVector() : GetActorRightVector(); }

	/* Setters */
	FORCEINLINE void SetHerdManagerOwner(AC_HerdManager* NewHerdManager) { HerdManagerOwner = NewHerdManager; }
	FORCEINLINE void SetPatrolTargets(TArray<AActor*> NewPatrolTargets) { PatrolTargets = NewPatrolTargets; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetRunSpeed(float Value) { RunSpeed = Value; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetCombatTarget(AActor* NewTarget) { CombatTarget = NewTarget; }

	UFUNCTION(BlueprintCallable)
	void SetMovementSpeed(EEnemyMovementSpeed MovementSpeed);

	//Function to check if enemy has already dies once from the checkpoint system
	UFUNCTION(BlueprintCallable)
	bool CheckIfEnemyHasDiedOnce();

protected: // Functions
	virtual void BeginPlay() override;
	virtual void Destroyed() override;

	UFUNCTION(BlueprintCallable)
	void InterruptBehaviorTreeLogic(const FString& Reason);

	UFUNCTION(BlueprintCallable)
	void ResumeBehaviorTreeLogic(const FString& Reason); 

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetHealth() { return PlayerAttributeSet->GetHealth(); }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetHealth(float NewValue) { PlayerAttributeSet->SetHealth(NewValue); }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void ResetPoise() { PlayerAttributeSet->SetPoise(0.0f); }

	UFUNCTION(BlueprintCallable, meta = (ExpandBoolAsExecs = "ReturnValue"))
	FORCEINLINE bool IsStunned() { return GetPoisePercentage() > 0.99; } 
	
	virtual void InitAbilityActorInfo() override;

protected: // Variables
	UPROPERTY(BlueprintReadOnly)
	bool bHealthBarVisible;

	UPROPERTY(BlueprintReadWrite)
	int32 LightAttackTracker;

	UPROPERTY(BlueprintReadWrite)
	int32 HeavyAttackTracker;

	UPROPERTY(BlueprintReadWrite)
	int32 RangedAttackTracker;

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Herd Manager Params") // Get a reference to Herd Manager owner so that Enemy can use its Combat Token pool. 
	TObjectPtr<AC_HerdManager> HerdManagerOwner;

	UPROPERTY(EditAnywhere, Category = "Animation Montages")
	TObjectPtr<UAnimMontage> PassiveToAttackTransitionAnimation;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Herd Setup", meta = (Categories = "MechBeast"))
	FGameplayTag MechBeastIdentityTag;

private: // Functions
	void InitializeEnemy();
	void InitializeAttributes();
	void UpdateWorldKnowledge();
	void TickCooldowns(float DeltaTime);

private: // Variables
	/* Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UC_HealthBarComponent> HealthBarWidget;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AC_AIController> EnemyAIController;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBlackboardComponent> Blackboard;

	UPROPERTY(VisibleInstanceOnly, Category = "Cooldowns")
	TMap<int32, float> TaskCooldowns;

	/* Herd Manager Parameters */
	UPROPERTY(EditAnywhere, Category = "Herd Manager Params") // Received from its Herd Manager (except for onboarding enemies). 
	TArray<TObjectPtr<AActor>> PatrolTargets;

	UPROPERTY(EditDefaultsOnly, Category = "Herd Manager Params")
	float XPToReward = 10.0f;

	/* Navigation */
	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WalkSpeed = 150.0f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation", meta = (AllowPrivateAccess = "true"))
	float RunSpeed = 300.0f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float FleeSpeed = 600.0f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float AttackRange = 100.0f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float StrafeRange = 300.0f;

	/* AI World Knowledge, Continuously Update Blackboard Keys */
	UPROPERTY(VisibleInstanceOnly, Category = "World Knowledge")
	TObjectPtr<AC_BasePlayer> Player;

	UPROPERTY(VisibleInstanceOnly, Category = "World Knowledge")
	FVector PlayerLocation;

	UPROPERTY(VisibleInstanceOnly, Category = "World Knowledge")
	float DistanceToPlayer;

	UPROPERTY(VisibleInstanceOnly, Category = "World Knowledge")
	bool bPlayerIsGrounded;

	UPROPERTY(VisibleInstanceOnly, Category = "World Knowledge")
	bool bPlayerIsDead;

	UPROPERTY(VisibleInstanceOnly, Category = "World Knowledge")
	bool bPlayerIsPhoenix;

	UPROPERTY(VisibleInstanceOnly, Category = "World Knowledge")
	bool bIsInFrontOfPlayer;

	UPROPERTY(VisibleInstanceOnly, Category = "World Knowledge")
	bool bCanSeePlayer;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Death", meta = (AllowPrivateAccess = "true"))
	bool bHasPlayedDeathEffect;

	FTimerHandle WorldKnowledgeHandle;
};