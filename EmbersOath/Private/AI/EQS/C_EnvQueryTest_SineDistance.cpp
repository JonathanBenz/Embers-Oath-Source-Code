// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz
// Most of this is copy-pasted from Epic's EnvQueryTestDistance implementation. The only difference is that the scoring function is changed to use a sine function. 

#include "AI/EQS/C_EnvQueryTest_SineDistance.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"

FORCEINLINE float CalcDistance3D(const FVector& PosA, const FVector& PosB) { return static_cast<float>(FVector::Distance(PosA, PosB)); }
FORCEINLINE float CalcDistance2D(const FVector& PosA, const FVector& PosB) { return static_cast<float>(FVector::Dist2D(PosA, PosB)); }
FORCEINLINE float CalcDistanceZ(const FVector& PosA, const FVector& PosB) { return static_cast<float>(PosB.Z - PosA.Z); }
FORCEINLINE float CalcDistanceAbsoluteZ(const FVector& PosA, const FVector& PosB) { return static_cast<float>(FMath::Abs(PosB.Z - PosA.Z)); }

UC_EnvQueryTest_SineDistance::UC_EnvQueryTest_SineDistance()
{
	DistanceTo = UEnvQueryContext_Querier::StaticClass();
	Cost = EEnvTestCost::Low;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
}

void UC_EnvQueryTest_SineDistance::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (!QueryOwner) return;

	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	float MinThresholdValue = FloatValueMin.GetValue();

	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);
	float MaxThresholdValue = FloatValueMax.GetValue();

	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(DistanceTo, ContextLocations)) return;

	switch (TestMode)
	{
	case EEnvTestDistance::Distance3D:
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
			{
				// Using a sine scoring function 
				const float Distance = CalcDistance3D(ItemLocation, ContextLocations[ContextIndex]);
				float NormalizedScore = (Distance - MinThresholdValue) / (MaxThresholdValue - MinThresholdValue);
				NormalizedScore = FMath::Clamp(NormalizedScore, 0.0f, 1.0f);
				const float SineScore = FMath::Sin(NormalizedScore * PI);
				It.SetScore(TestPurpose, FilterType, SineScore, 0.0f, 1.0f);
			}
		}
		break;

	case EEnvTestDistance::Distance2D:
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
			{
				// Using a sine scoring function 
				const float Distance = CalcDistance2D(ItemLocation, ContextLocations[ContextIndex]);
				float NormalizedScore = (Distance - MinThresholdValue) / (MaxThresholdValue - MinThresholdValue);
				NormalizedScore = FMath::Clamp(NormalizedScore, 0.0f, 1.0f);
				const float SineScore = FMath::Sin(NormalizedScore * PI);
				It.SetScore(TestPurpose, FilterType, SineScore, 0.0f, 1.0f);
			}
		}
		break;

	case EEnvTestDistance::DistanceZ:
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
			{
				// Using a sine scoring function 
				const float Distance = CalcDistanceZ(ItemLocation, ContextLocations[ContextIndex]);
				float NormalizedScore = (Distance - MinThresholdValue) / (MaxThresholdValue - MinThresholdValue);
				NormalizedScore = FMath::Clamp(NormalizedScore, 0.0f, 1.0f);
				const float SineScore = FMath::Sin(NormalizedScore * PI);
				It.SetScore(TestPurpose, FilterType, SineScore, 0.0f, 1.0f);
			}
		}
		break;

	case EEnvTestDistance::DistanceAbsoluteZ:
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			const FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
			{
				// Using a sine scoring function 
				const float Distance = CalcDistanceAbsoluteZ(ItemLocation, ContextLocations[ContextIndex]);
				float NormalizedScore = (Distance - MinThresholdValue) / (MaxThresholdValue - MinThresholdValue);
				NormalizedScore = FMath::Clamp(NormalizedScore, 0.0f, 1.0f);
				const float SineScore = FMath::Sin(NormalizedScore * PI);
				It.SetScore(TestPurpose, FilterType, SineScore, 0.0f, 1.0f);
			}
		}
		break;

	default:
		checkNoEntry();
		return;
	}
}

FText UC_EnvQueryTest_SineDistance::GetDescriptionTitle() const
{
	FString ModeDesc;
	switch (TestMode)
	{
	case EEnvTestDistance::Distance3D:
		ModeDesc = TEXT("");
		break;

	case EEnvTestDistance::Distance2D:
		ModeDesc = TEXT(" 2D");
		break;

	case EEnvTestDistance::DistanceZ:
		ModeDesc = TEXT(" Z");
		break;

	default:
		break;
	}

	return FText::FromString(FString::Printf(TEXT("%s%s: to %s"), *Super::GetDescriptionTitle().ToString(), *ModeDesc, *UEnvQueryTypes::DescribeContext(DistanceTo).ToString()));
}

FText UC_EnvQueryTest_SineDistance::GetDescriptionDetails() const { return DescribeFloatTestParams(); }