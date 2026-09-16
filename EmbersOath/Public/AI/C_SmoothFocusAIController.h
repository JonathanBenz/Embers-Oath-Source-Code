// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "C_SmoothFocusAIController.generated.h"

UCLASS()
class EMBERSOATH_API AC_SmoothFocusAIController : public AAIController
{
	GENERATED_BODY()

public:
	virtual void UpdateControlRotation(float DeltaTime, bool bUpdatePawn) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SmoothFocusInterpSpeed = 30.0f;

private:
	FRotator SmoothTargetRotation;
};