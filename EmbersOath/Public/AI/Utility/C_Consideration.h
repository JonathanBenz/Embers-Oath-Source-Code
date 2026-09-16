// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "C_Consideration.generated.h"

UENUM(BlueprintType)
enum class EUtilityInputType : uint8
{
	DistanceToPlayer,
	MyHealth,
	HealthOfPlayer,
	//MySpeed,
	//MyPoise,
	//TimesUsedLightAttack,
	//TimesUsedHeavyAttack,
	//TimesUsedRangedAttack,
	InFrontOfPlayer, 
	CanSeePlayer,
	PlayerIsGrounded,
	PlayerIsPhoenix,
	AllyCount
};

class AC_BaseEnemy;
class UCurveFloat;

USTRUCT(BlueprintType)
struct EMBERSOATH_API FConsideration
{
	GENERATED_BODY()
	
public:
	float EvaluateConsiderationScore(AC_BaseEnemy* AIAgent) const;

	UPROPERTY(EditAnywhere, Category = "Utility Params")
	EUtilityInputType InputType = EUtilityInputType::DistanceToPlayer;

	UPROPERTY(EditAnywhere, Category = "Utility Params")
	float MinValue = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Utility Params")
	float MaxValue = FLT_MAX;

	UPROPERTY(EditAnywhere, Category = "Utility Params")
	TObjectPtr<UCurveFloat> ResponseCurve;

private:
	float GetNormalizedInput(AC_BaseEnemy* AIAgent) const;
	float Normalize(float Value) const;
	float ApplyResponseCurve(float X) const;
	float Sanitize(float Y) const;
};