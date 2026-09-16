// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz
// Most of this is copy-pasted from Epic's EnvQueryTestDistance implementation. The only difference is that the scoring function is changed to use a sine function. 

#pragma once
#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "Templates/SubclassOf.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvironmentQuery/Tests/EnvQueryTest_Distance.h"
#include "C_EnvQueryTest_SineDistance.generated.h"

UCLASS()
class EMBERSOATH_API UC_EnvQueryTest_SineDistance : public UEnvQueryTest
{
	GENERATED_BODY()
	
public:
	UC_EnvQueryTest_SineDistance();

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;

	UPROPERTY(EditDefaultsOnly, Category = Distance)
	TEnumAsByte<EEnvTestDistance::Type> TestMode;

	UPROPERTY(EditDefaultsOnly, Category = Distance)
	TSubclassOf<UEnvQueryContext> DistanceTo;
};