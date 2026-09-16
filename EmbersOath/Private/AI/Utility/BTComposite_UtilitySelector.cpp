// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "AI/Utility/BTComposite_UtilitySelector.h"
#include "AI/Utility/BTTask_UtilityBase.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTComposite_UtilitySelector::UBTComposite_UtilitySelector(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Utility Selector";
}

/// <summary>
/// Loop through all Utility Task children nodes, keep track of the highest calculated Utility score and task, and cache that data into the agent's Blackboard.
/// </summary>
int32 UBTComposite_UtilitySelector::GetNextChildHandler(FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const
{
	int32 NextChildIdx = BTSpecialChild::ReturnToParent;

	// If evaluating from the first child
	if (PrevChild == BTSpecialChild::NotInitialized)
	{
		// newly activated: start from first
		NextChildIdx = 0;

		// Look ahead at every Utility Task and cache which one has the best Utility score. 
		// There will be a decorator that filters exclusively for this cached value to allow execution. 
		FindChildWithBestUtilityScore(SearchData);
	}
	// Keep iterating until you reach the best Utility Task (all other nodes should return failure due to the decorator). 
	else if (LastResult == EBTNodeResult::Failed && (PrevChild + 1) < GetChildrenNum())
	{
		// failed = choose next child
		NextChildIdx = PrevChild + 1;
	}

	return NextChildIdx;
}

void UBTComposite_UtilitySelector::FindChildWithBestUtilityScore(FBehaviorTreeSearchData& SearchData) const
{
	// Loop through every child node to find which one has the best Utility score. 
	float BestUtility = -1.0f;
	int32 BestChildTaskID = -1;
	for (int32 i = 0; i < GetChildrenNum(); i++)
	{
		const FBTCompositeChild& ChildInfo = Children[i];
		if (UBTTask_UtilityBase* TaskNode = Cast<UBTTask_UtilityBase>(ChildInfo.ChildTask)) // This gives us the template of the task node. 
		{
			// Check if node template is instanced. If so, grab the instance of the node instead. For blueprint nodes, this SHOULD be the case. 
			if (TaskNode->HasInstance())
			{
				uint8* NodeMemory = SearchData.OwnerComp.GetNodeMemory(TaskNode, SearchData.OwnerComp.GetActiveInstanceIdx());
				if (UBTNode* InstanceNode = TaskNode->GetNodeInstance(SearchData.OwnerComp, NodeMemory))
					TaskNode = Cast<UBTTask_UtilityBase>(InstanceNode);
			}
			
			// Additional safety check in case the node instance cast failed. 
			if (TaskNode)
			{
				// Calculate the total Utility Score of each task. 
				const float Score = TaskNode->CalculateUtilityScore(SearchData.OwnerComp);

				if (Score > BestUtility)
				{
					BestUtility = Score;
					BestChildTaskID = TaskNode->GetTaskID();
				}
			}
		}
	}

	// Cache the highest Utility score and its respective Task to the Blackboard as Blackboard Keys. 
	if (UBlackboardComponent* BlackboardComponent = SearchData.OwnerComp.GetBlackboardComponent())
	{
		BlackboardComponent->SetValueAsFloat(FName("LocalBestUtilityScore"), BestUtility);
		BlackboardComponent->SetValueAsInt(FName("LocalBestTaskID"), BestChildTaskID);
	}
}