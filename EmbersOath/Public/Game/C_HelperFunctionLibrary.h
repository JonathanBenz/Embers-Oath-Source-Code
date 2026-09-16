// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Game/C_Enums.h"
#include "ShaderPipelineCache.h"
#include "C_HelperFunctionLibrary.generated.h"

UCLASS()
class EMBERSOATH_API UC_HelperFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	/** Convert enum to actual level name */
	UFUNCTION(BlueprintPure, Category = "Levels")
	static inline FName GetLevelNameFromEnum(ELevelNames LevelName)
	{
		switch (LevelName)
		{
			case ELevelNames::MainMenu:			return FName("M_MainMenu");
			case ELevelNames::HiddenSettlement:	return FName("M_HiddenSettlement");
			default:							return NAME_None;
		}
	}

	// PSO Helper Functions
	UFUNCTION(BlueprintCallable, Category = "PSO")
	static inline void StartFastPSOCompilation() 
	{
		FShaderPipelineCache::SetBatchMode(FShaderPipelineCache::BatchMode::Fast);
		FShaderPipelineCache::ResumeBatching();
	}

	UFUNCTION(BlueprintPure, Category = "PSO")
	static inline int32 GetRemainingPSOCompiles() 
	{ 
		return static_cast<int32>(FShaderPipelineCache::NumPrecompilesRemaining()); 
	}

	UFUNCTION(BlueprintCallable, Category = "PSO")
	static inline void SetPSOCompilationBackground() 
	{ 
		FShaderPipelineCache::SetBatchMode(FShaderPipelineCache::BatchMode::Background); 
	}
};