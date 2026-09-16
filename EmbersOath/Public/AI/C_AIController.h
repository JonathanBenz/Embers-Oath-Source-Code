// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "DetourCrowdAIController.h"
#include "C_AIController.generated.h"

class UBehaviorTree;
class AC_BaseEnemy;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Damage;

UCLASS()
class EMBERSOATH_API AC_AIController : public ADetourCrowdAIController
{
	GENERATED_BODY()
	
public:
	AC_AIController();

	UFUNCTION(BlueprintCallable)
	void StartSettingStateToAttack(AActor* AttackTarget);

	UFUNCTION(BlueprintCallable)
	void ImmediatelySetStateToAttack(AActor* AttackTarget);

	void EnemyPlayPassiveToAttackTransitionAnimation();

protected:
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable)
	void SetStateAsPassive();

	UFUNCTION(BlueprintCallable)
	void FinishSettingStateToAttack();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UAIPerceptionComponent> AIPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<AActor> CombatTarget;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AC_BaseEnemy> EnemyPawn;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBehaviorTree> AIBehaviorTree;

	/* AI Perception */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UAISenseConfig_Damage> DamageConfig;

	UPROPERTY(EditAnywhere)
	float MinDelay = 0.2f;

	UPROPERTY(EditAnywhere)
	float MaxDelay = 1.0f;

	FTimerHandle RandomlyDelayPlayAnimationTimerHandle;
};