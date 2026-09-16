// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "AI/Tasks/BTTask_SetMovementSpeed.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/C_AIController.h"
#include "Characters/Enemies/C_BaseEnemy.h"

UBTTask_SetMovementSpeed::UBTTask_SetMovementSpeed()
{
	NodeName = TEXT("Set Movement Speed");
}

EBTNodeResult::Type UBTTask_SetMovementSpeed::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AC_BaseEnemy* Enemy = Cast<AC_BaseEnemy>(AIController->GetPawn());
	if (!Enemy || !AIController) return EBTNodeResult::Failed;

	if (EnemyState == EEnemyState::EES_Patrolling)
		Enemy->GetCharacterMovement()->MaxWalkSpeed = Enemy->GetWalkSpeed();
	else if (EnemyState == EEnemyState::EES_Attacking)
		Enemy->GetCharacterMovement()->MaxWalkSpeed = Enemy->GetRunSpeed();
	else if (EnemyState == EEnemyState::EES_Fleeing)
		Enemy->GetCharacterMovement()->MaxWalkSpeed = Enemy->GetFleeSpeed();
	else
		Enemy->GetCharacterMovement()->MaxWalkSpeed = 0;

	return EBTNodeResult::Succeeded;
}