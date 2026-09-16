// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "AI/Tasks/BTTask_SetCombatTarget.h"
#include "AI/C_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/C_BaseEnemy.h"
#include "Kismet/GameplayStatics.h"

UBTTask_SetCombatTarget::UBTTask_SetCombatTarget()
{
	NodeName = TEXT("Set Combat Target");
}

EBTNodeResult::Type UBTTask_SetCombatTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	AC_BaseEnemy* Enemy = Cast<AC_BaseEnemy>(AIController->GetPawn());
	if (!Enemy || !AIController) return EBTNodeResult::Failed;

	TObjectPtr<AActor> PreviousCombatTarget = Enemy->GetCombatTarget();
	TObjectPtr<AActor> CurrentCombatTarget = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!CurrentCombatTarget) return EBTNodeResult::Failed;

	// If PreviousCombatTarget is not initialized, or if the Current and Previous target are different, then update the Enemy's target. 
	if (!PreviousCombatTarget || CurrentCombatTarget != PreviousCombatTarget)
		Enemy->SetCombatTarget(CurrentCombatTarget);

	OwnerComp.GetBlackboardComponent()->SetValueAsObject(GetSelectedBlackboardKey(), CurrentCombatTarget);
	return EBTNodeResult::Succeeded;
}