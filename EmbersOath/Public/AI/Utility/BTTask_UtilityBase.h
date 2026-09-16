// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_UtilityBase.generated.h"

struct FConsideration;
class AC_BaseEnemy;

UCLASS(Abstract, Blueprintable)
class EMBERSOATH_API UBTTask_UtilityBase : public UBTTask_BlueprintBase
{
	GENERATED_BODY()
	
public:
    UBTTask_UtilityBase(const FObjectInitializer& ObjectInitializer);

    // Called by Utility Selector Composite Node. 
	float CalculateUtilityScore(UBehaviorTreeComponent& OwnerComp, bool bLogMessages = false);

    UFUNCTION(BlueprintCallable, meta = (ExpandBoolAsExecs = "ReturnValue"))
    bool CheckForValidUtilityScore(AC_BaseEnemy* EnemyAgent);

    UFUNCTION(BlueprintCallable)
    void ResetCanAttackFlag(AC_BaseEnemy* EnemyAgent);

    UFUNCTION(BlueprintCallable)
    void StartTaskCooldown(AC_BaseEnemy* EnemyAgent);

    FORCEINLINE float GetCooldownTime() { return BaseCooldownTime + FMath::RandRange(-DeltaPlusMinusRange, DeltaPlusMinusRange); }
    FORCEINLINE int32 GetTaskID() { return UtilityTaskID; }

    virtual void DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const override;
    virtual FString GetStaticDescription() const override;

    // These functions should only be overriden for non-instanced task nodes. All blueprint task nodes ARE instanced however, so this has no use. 
    //virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
    //virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;
    //virtual uint16 GetInstanceMemorySize() const override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector bAllowedToAttackKey;

    UPROPERTY(EditAnywhere, Category = "Utility")
    int32 UtilityTaskID = -1;

    // Max of 8 considerations in this array.
    UPROPERTY(EditAnywhere, Category = "Utility", meta = (TitleProperty = "InputType"))
    TArray<FConsideration> Considerations;

    UPROPERTY(EditAnywhere, Category = "Utility", meta = (UIMin = 0, ClampMin = 0))
    float BaseCooldownTime = 0.0f;

    // Give a +/- random offset to the CooldownTime. 
    UPROPERTY(EditAnywhere, Category = "Utility", meta = (UIMin = 0, ClampMin = 0))
    float DeltaPlusMinusRange = 0.0f;

    // The final utility score is multiplied by this weight. This allows for the score to become greater than 1.0f, if desired.  
    UPROPERTY(EditAnywhere, Category = "Utility", meta = (UIMin = 0, ClampMin = 0))
    float Weight = 1.0f;

    /* Runtime Data */
    bool bIsOnCooldown = false;
    int32 NumConsiderations = 0;
    float FinalScore = 0.0f;

    // The size of ConsiderationScores array is 8. This is an arbitrary value. I doubt a task would exceed having this amount of considerations. 
    float ConsiderationScores[8] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };
    /* End Runtime Data */
};

// NodeMemory is only useful for owning small chunks of data for non-instanced task nodes. 
// All blueprint task nodes ARE instanced however, so this has no use. 
// We can just take these data and put them directly as member variables of this header. 
/*
struct FBTUtilityMemory
{
    bool bIsOnCooldown = false;
    int32 NumConsiderations = 0;
    float FinalScore = 0.0f;
    float ConsiderationScores[8] = { -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f };
};
*/