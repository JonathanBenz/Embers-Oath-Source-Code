// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "AI/Utility/BTTask_UtilityBase.h"
#include "AI/Utility/C_Consideration.h"
#include "AI/Utility/BTComposite_UtilitySelector.h"
#include "AI/C_AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/Enemies/C_BaseEnemy.h"

UBTTask_UtilityBase::UBTTask_UtilityBase(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { }

/// <summary>
/// This function is called by the Utility Selector to see which Utility Task scores the highest. 
/// In essence, each consideration is evaluated and the geometric mean (https://en.wikipedia.org/wiki/Geometric_mean) (e.g., the math part)
/// of all these consideration scores determines this Utility Task node's final Utility score. 
/// </summary>
float UBTTask_UtilityBase::CalculateUtilityScore(UBehaviorTreeComponent& OwnerComp, bool bLogMessages)
{
	//FBTUtilityMemory* NodeMemory = CastInstanceNodeMemory<FBTUtilityMemory>(OwnerComp.GetNodeMemory(this, OwnerComp.GetActiveInstanceIdx()));

	AC_BaseEnemy* AIAgent = Cast<AC_BaseEnemy>(OwnerComp.GetAIOwner()->GetPawn());
	if (!AIAgent)
	{
		if (bLogMessages) UE_LOG(LogTemp, Error, TEXT("UtilityTask ERROR: No reference to agent!"));
		return 0.0f;
	}
	if (Considerations.IsEmpty())
	{
		if (bLogMessages) UE_LOG(LogTemp, Error, TEXT("UtilityTask ERROR: No considerations were added!"));
		return 0.0f;
	}
	if (AIAgent->IsTaskOnCooldown(UtilityTaskID))
	{
		if (bLogMessages) UE_LOG(LogTemp, Display, TEXT("%s has Task %d is on cooldown."), *AIAgent->GetName(), UtilityTaskID);
		//NodeMemory->bIsOnCooldown = true;
		bIsOnCooldown = true;
		return 0.0f; // If this task is on cooldown, automatically return a score of 0.0f.
	}
	//NodeMemory->bIsOnCooldown = false;
	bIsOnCooldown = false;

	// Calculate the Geometric Mean of all Consideration scores. 
	float CumulativeUtility = 0.0f;
	int32 ConsiderationIdx = 0;
	for (const FConsideration Consideration : Considerations)
	{
		const float Score = Consideration.EvaluateConsiderationScore(AIAgent);

		// The size of ConsiderationScores array is 8. This is an arbitrary value. I doubt a task would exceed having this amount of considerations. 
		if (ConsiderationIdx < 8)
		{
			//NodeMemory->ConsiderationScores[ConsiderationIdx] = Score;
			ConsiderationScores[ConsiderationIdx] = Score;
		}

		if (Score <= 0.0f) // Return early due to multiplication with zero.
		{
			if (bLogMessages) UE_LOG(LogTemp, Display, TEXT("A consideration evaluated with a score of 0."));
			//NodeMemory->NumConsiderations = ConsiderationIdx + 1;
			//NodeMemory->FinalScore = 0.0f;
			NumConsiderations = ConsiderationIdx + 1;
			FinalScore = 0.0f;
			return 0.0f;
		}

		CumulativeUtility += FMath::Log2(Score);
		ConsiderationIdx++;
	}

	float m = CumulativeUtility / static_cast<float>(ConsiderationIdx);
	float FinalUtilityScore = FMath::Pow(2, m);
	FinalUtilityScore *= Weight;

	//NodeMemory->NumConsiderations = ConsiderationIdx;
	//NodeMemory->FinalScore = FinalUtilityScore;
	NumConsiderations = ConsiderationIdx;
	FinalScore = FinalUtilityScore;

	return FinalUtilityScore;
}

/// <summary>
/// Return true if UtilityScore is a non-zero value. Else, returns false. 
/// This should be called at the Start of any Utility Task node, and is a BlueprintCallable function.
/// </summary>
bool UBTTask_UtilityBase::CheckForValidUtilityScore(AC_BaseEnemy* EnemyAgent)
{
	return FinalScore > 0.0f;
}

/// <summary>
/// Resets the bCanAttack blackboard boolean key back to false.
/// This should be called at the End of any Utility Task node, and is a BlueprintCallable function. 
/// </summary>
void UBTTask_UtilityBase::ResetCanAttackFlag(AC_BaseEnemy* EnemyAgent)
{
	EnemyAgent->GetEnemyBlackboard()->SetValueAsBool(bAllowedToAttackKey.SelectedKeyName, false);
}

/// <summary>
/// Adds or resets this Utility Task into the EnemyAgent's cooldown map. 
/// Will cause this Utility Task to automatically evaluate its Utility Score to 0.0f as long as its on cooldown. 
/// This should be called at the End of any Utility Task node, and is a BlueprintCallable function. 
/// </summary>
void UBTTask_UtilityBase::StartTaskCooldown(AC_BaseEnemy* EnemyAgent)
{
	EnemyAgent->StartTaskCooldown(GetTaskID(), GetCooldownTime());
}

/// <summary>
/// Reads the values from FBTUtilityMemory (which are modified within the CalculateUtilityScore() function) and displays them at the bottom of the node during runtime. 
/// Very useful for debugging Utility scores during PIE! 
/// </summary>
void UBTTask_UtilityBase::DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const
{
	Super::DescribeRuntimeValues(OwnerComp, NodeMemory, Verbosity, Values);
	Values.Add(FString::Printf(TEXT("\n")));

	//FBTUtilityMemory* TaskMemory = (FBTUtilityMemory*)NodeMemory;
	if (bIsOnCooldown /*TaskMemory->bIsOnCooldown*/)
	{
		Values.Add(FString::Printf(TEXT("Utility Task Is On Cooldown")));
		return;
	}
	
	Values.Add(FString::Printf(TEXT("Final Utility Score: %s"), *FString::SanitizeFloat(FinalScore /*TaskMemory->FinalScore*/)));
	Values.Add(FString::Printf(TEXT("Num Considerations Evaluated: %d"), NumConsiderations /*TaskMemory->NumConsiderations*/));

	for (int32 i = 0; i < NumConsiderations /*TaskMemory->NumConsiderations*/; i++)
	{
		if (ConsiderationScores[i] /*TaskMemory->ConsiderationScores[i]*/ > -1.0f)
			Values.Add(FString::Printf(TEXT("\tC%d: %s"), i, *FString::SanitizeFloat(ConsiderationScores[i] /*TaskMemory->ConsiderationScores[i]*/)));
	}
}

FString UBTTask_UtilityBase::GetStaticDescription() const
{
	return FString::Printf(TEXT("%s \nUtility Task ID: %d \nCooldown Time: %s s +- %s s"), 
		*Super::GetStaticDescription(), UtilityTaskID, *FString::SanitizeFloat(BaseCooldownTime), *FString::SanitizeFloat(DeltaPlusMinusRange));
}

// These functions should only be defined for non-instanced task nodes. All blueprint task nodes ARE instanced however, so this has no use. 
/*
void UBTTask_UtilityBase::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	InitializeNodeMemory<FBTUtilityMemory>(NodeMemory, InitType);
}

void UBTTask_UtilityBase::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
	CleanupNodeMemory<FBTUtilityMemory>(NodeMemory, CleanupType);
}

uint16 UBTTask_UtilityBase::GetInstanceMemorySize() const
{
	return sizeof(FBTUtilityMemory);
}
*/