// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "C_DialogueInteraction.generated.h"

class USoundWave;

USTRUCT(BlueprintType)
struct FDialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Text;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<USoundWave> VoiceOver;
};

UCLASS(BlueprintType)
class EMBERSOATH_API UC_DialogueInteraction : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName InteractionId;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FDialogueLine> Lines;
};