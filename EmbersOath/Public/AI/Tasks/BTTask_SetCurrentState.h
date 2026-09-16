// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "Characters/C_CharacterTypes.h"
#include "BTTask_SetCurrentState.generated.h"

UCLASS()
class EMBERSOATH_API UBTTask_SetCurrentState : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

public:
	UBTTask_SetCurrentState();

protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	EEnemyState EnemyState;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};