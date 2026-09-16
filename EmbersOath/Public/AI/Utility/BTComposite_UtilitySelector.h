// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BTComposite_UtilitySelector.generated.h"

UCLASS(MinimalAPI)
class UBTComposite_UtilitySelector : public UBTCompositeNode
{
	GENERATED_BODY()

public:
	UBTComposite_UtilitySelector(const FObjectInitializer& ObjectInitializer);

protected:
	virtual int32 GetNextChildHandler(struct FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const override;
	void FindChildWithBestUtilityScore(FBehaviorTreeSearchData& SearchData) const;
};