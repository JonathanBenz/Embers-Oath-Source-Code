// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "AI/Utility/C_Consideration.h"
#include "AI/C_HerdManager.h"
#include "Characters/Enemies/C_BaseEnemy.h"
#include "Characters/C_BasePlayer.h"
#include "Curves/CurveFloat.h"

/// <summary>
/// To evaluate this consideration's score, its Input Parameter is normalized and then sent to the Response Curve. 
/// </summary>
/// <param name="AIAgent"> The AIAgent who is being evaluated. </param>
/// <returns> The output of the Response Curve, which will be a normalized value in the range [0, 1]. </returns>
float FConsideration::EvaluateConsiderationScore(AC_BaseEnemy* AIAgent) const
{
	if (!AIAgent) return 0.0f;

	float NormalizedInput = GetNormalizedInput(AIAgent);
	float FinalScore = ApplyResponseCurve(NormalizedInput);
	return FinalScore;
}

/// <summary>
/// This method acts as our Input Clearing House. 
/// </summary>
float FConsideration::GetNormalizedInput(AC_BaseEnemy* AIAgent) const
{
	switch (InputType)
	{
		case EUtilityInputType::DistanceToPlayer:
		{
			const float Dist = AIAgent->GetDistanceToPlayer();
			return Normalize(Dist);
		}

		case EUtilityInputType::MyHealth:
		{
			const float Current = AIAgent->PlayerAttributeSet->GetHealth();
			const float Max = AIAgent->PlayerAttributeSet->GetMaxHealth();
			if (MaxValue < Max) return Normalize(Current); 
			else return Current / Max;
		}

		case EUtilityInputType::HealthOfPlayer:
		{
			const AC_BasePlayer* Player = AIAgent->GetPlayer();
			if (!Player || !Player->PlayerAttributeSet) return 0.0f;

			const float Current = Player->PlayerAttributeSet->GetHealth();
			const float Max = Player->PlayerAttributeSet->GetMaxHealth();
			if (MaxValue < Max) return Normalize(Current);
			else return Current / Max;
		}

		//case EUtilityInputType::MySpeed:
		//{
		//	const float Current = AIAgent->GetCurrentAgentSpeed();
		//	const float Max = AIAgent->GetMaxSpeed();
		//	if (MaxValue < Max) return Normalize(Current);
		//	else return Current / Max;
		//}

		//case EUtilityInputType::MyPoise:
		//{
		//	return AIAgent->GetPoisePercentage(); // This one is already normalized
		//}

		//case EUtilityInputType::TimesUsedLightAttack:
		//{
		//	const float Count = static_cast<float>(AIAgent->GetTimesUsedLightAttack());
		//	return Normalize(Count);
		//}

		case EUtilityInputType::InFrontOfPlayer:
			return AIAgent->IsInFrontOfPlayer() ? 1.0f : 0.0f;

		case EUtilityInputType::CanSeePlayer:
			return AIAgent->CanSeePlayer() ? 1.0f : 0.0f;

		case EUtilityInputType::PlayerIsGrounded:
			return AIAgent->PlayerIsGrounded() ? 1.0f : 0.0f;

		case EUtilityInputType::PlayerIsPhoenix:
			return AIAgent->PlayerIsPhoenix() ? 1.0f : 0.0f;

		case EUtilityInputType::AllyCount:
		{
			const float Current = static_cast<float>(AIAgent->GetHerdManager()->GetEnemyCount());
			const float Max = static_cast<float>(AIAgent->GetHerdManager()->GetInitialEnemyCount());
			if (MaxValue < Max) return Normalize(Current);
			else return Current / Max;
		}
	}

	return 0.0f;
}

/// <summary>
/// Helper function to normalize input. 
/// </summary>
float FConsideration::Normalize(float Value) const
{
	if (MaxValue <= MinValue) { UE_LOG(LogTemp, Error, TEXT("Error: Consideration has a MaxValue less than MinValue.")); return 0.0f; }
	return FMath::Clamp((Value - MinValue) / (MaxValue - MinValue), 0.0f, 1.0f);
}

/// <summary>
/// The normalized input is sent here to get the final consideration score.
/// </summary>
float FConsideration::ApplyResponseCurve(float X) const
{
	if (!ResponseCurve) { UE_LOG(LogTemp, Error, TEXT("Error: Consideration does NOT have a response curve.")); return 0.0f; }

	return Sanitize(ResponseCurve->GetFloatValue(X));
}

/// <summary>
/// Account for edge cases to ensure values are still within [0,1] domain. 
/// </summary>
float FConsideration::Sanitize(float Y) const
{
	if (!FMath::IsFinite(Y)) return 0.0f;
	if (Y < 0.0f) return 0.0f;
	if (Y > 1.0f) return 1.0f;
	return Y;
}