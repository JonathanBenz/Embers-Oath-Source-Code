// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz
// Most of this is copy-pasted from Epic's EnvQueryTestDot implementation. The only difference is that the scoring function is changed to use a sine function. 

#pragma once
#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "Templates/SubclassOf.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "C_EnvQueryTest_SineDot.generated.h"

UENUM(BlueprintType)
enum class EEnvTestDotAmogus : uint8
{
	Dot3D UMETA(DisplayName = "Dot (3D)", Tooltip = "Fully 3D dot-product"),
	Dot2D UMETA(DisplayName = "Dot 2D (Heading)", Tooltip = "Dot Product in the XY-plane, which is equivalent to the cosine of the heading or yaw angle.")
};

UCLASS()
class EMBERSOATH_API UC_EnvQueryTest_SineDot : public UEnvQueryTest
{
	GENERATED_BODY()
	
public:
	UC_EnvQueryTest_SineDot();
	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = Dot)
	FEnvDirection LineA;

	UPROPERTY(EditDefaultsOnly, Category = Dot)
	FEnvDirection LineB;

	UPROPERTY(EditDefaultsOnly, Category = Dot)
	EEnvTestDotAmogus TestMode;

	UPROPERTY(EditDefaultsOnly, Category = Dot)
	bool bAbsoluteValue;

	void GatherLineDirections(TArray<FVector>& Directions, FEnvQueryInstance& QueryInstance, const FVector& ItemLocation,
		TSubclassOf<UEnvQueryContext> LineFrom, TSubclassOf<UEnvQueryContext> LineTo) const;

	void GatherLineDirections(TArray<FVector>& Directions, FEnvQueryInstance& QueryInstance, const FRotator& ItemRotation,
		TSubclassOf<UEnvQueryContext> LineDirection) const;

	void GatherLineDirections(TArray<FVector>& Directions, FEnvQueryInstance& QueryInstance,
		TSubclassOf<UEnvQueryContext> LineFrom, TSubclassOf<UEnvQueryContext> LineTo, TSubclassOf<UEnvQueryContext> LineDirection, bool bUseDirectionContext,
		const FVector& ItemLocation = FVector::ZeroVector, const FRotator& ItemRotation = FRotator::ZeroRotator) const;

	bool RequiresPerItemUpdates(TSubclassOf<UEnvQueryContext> LineFrom, TSubclassOf<UEnvQueryContext> LineTo, TSubclassOf<UEnvQueryContext> LineDirection, bool bUseDirectionContext) const;
};