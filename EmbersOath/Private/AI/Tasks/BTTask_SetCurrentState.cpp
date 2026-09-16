// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "AI/Tasks/BTTask_SetCurrentState.h"
#include "AI/C_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/C_BaseEnemy.h"

UBTTask_SetCurrentState::UBTTask_SetCurrentState()
{
	NodeName = TEXT("Set Current State");
}

EBTNodeResult::Type UBTTask_SetCurrentState::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AC_BaseEnemy* Enemy = Cast<AC_BaseEnemy>(AIController->GetPawn());
	if (!Enemy || !AIController) return EBTNodeResult::Failed;

	uint8 byte = static_cast<uint8>(EnemyState);
	OwnerComp.GetBlackboardComponent()->SetValueAsEnum(GetSelectedBlackboardKey(), byte);
	return EBTNodeResult::Succeeded;
}