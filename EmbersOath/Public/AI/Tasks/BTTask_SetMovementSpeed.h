// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Characters/C_CharacterTypes.h"
#include "BTTask_SetMovementSpeed.generated.h"

UCLASS()
class EMBERSOATH_API UBTTask_SetMovementSpeed : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_SetMovementSpeed();

protected:
	UPROPERTY(EditAnywhere, Category = "AI")
	EEnemyState EnemyState;

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};