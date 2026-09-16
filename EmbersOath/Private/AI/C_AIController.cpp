// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "AI/C_AIController.h"
#include "Characters/Enemies/C_BaseEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/C_CharacterTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"

AC_AIController::AC_AIController()
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(FName("AI Perception"));
	DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(FName("Damage Config"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(FName("Sight Config"));
	SightConfig->PeripheralVisionAngleDegrees = 60.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->ConfigureSense(*DamageConfig);
}

void AC_AIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (AC_BaseEnemy* Enemy = Cast<AC_BaseEnemy>(InPawn))
	{
		AIBehaviorTree = Enemy->GetBehaviorTree();
		EnemyPawn = Enemy;
		RunBehaviorTree(AIBehaviorTree);
		GetBlackboardComponent()->SetValueAsFloat(FName("AttackRadius"), Enemy->GetAttackRange());
		GetBlackboardComponent()->SetValueAsFloat(FName("StrafeRadius"), Enemy->GetStrafeRange());
		SetStateAsPassive();
	}
}

void AC_AIController::SetStateAsPassive()
{
	uint8 State = static_cast<uint8>(EEnemyState::EES_Patrolling);
	GetBlackboardComponent()->SetValueAsEnum(FName("EnemyState"), State);
}

/// <summary>
/// Set the Player Blackboard key, set the enemy's Combat Target (needed for motion warping during attacks), and then play a transition animation (such as barking or roaring). 
/// </summary>
/// <param name="AttackTarget"></param>
void AC_AIController::StartSettingStateToAttack(AActor* AttackTarget)
{
	GetBlackboardComponent()->SetValueAsObject(FName("Player"), AttackTarget);
	SetFocus(AttackTarget);

	EnemyPawn->SetCombatTarget(AttackTarget); // Set C_BaseEnemy CombatTarget
	const float RandomDelay = FMath::FRandRange(MinDelay, MaxDelay);
	GetWorldTimerManager().SetTimer(RandomlyDelayPlayAnimationTimerHandle, this, &AC_AIController::EnemyPlayPassiveToAttackTransitionAnimation, RandomDelay);
	CombatTarget = AttackTarget; // Set this AI Controller's CombatTarget
}

/// <summary>
/// Similar to the StartSettingStateToAttack() function, except it skips the transition animation. 
/// </summary>
/// <param name="CombatTarget"></param>
void AC_AIController::ImmediatelySetStateToAttack(AActor* AttackTarget)
{
	GetBlackboardComponent()->SetValueAsObject(FName("Player"), AttackTarget);
	SetFocus(AttackTarget);
	EnemyPawn->SetCombatTarget(AttackTarget); // Set C_BaseEnemy CombatTarget
	CombatTarget = AttackTarget; // Set this AI Controller's CombatTarget
	const float RandomDelay = FMath::FRandRange(MinDelay, MaxDelay);
	GetWorldTimerManager().SetTimer(RandomlyDelayPlayAnimationTimerHandle, this, &AC_AIController::FinishSettingStateToAttack, RandomDelay); // Small reaction time timer
}

/// <summary>
/// In the transition animation, call this at the end when reaching the FinishTransition anim notify event. 
/// </summary>
void AC_AIController::FinishSettingStateToAttack()
{
	uint8 State = static_cast<uint8>(EEnemyState::EES_Attacking);
	GetBlackboardComponent()->SetValueAsEnum(FName("EnemyState"), State);
}

void AC_AIController::EnemyPlayPassiveToAttackTransitionAnimation()
{
	EnemyPawn->PlayPassiveToAttackTransitionAnimation();
}