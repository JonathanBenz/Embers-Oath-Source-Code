// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_DialogueWidget.generated.h"

class UTextBlock;
struct FDialogueLine;

UCLASS()
class EMBERSOATH_API UC_DialogueWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void UpdateTextBlocks(const FDialogueLine& DialogueLine);

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> Speaker;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> Dialogue;
};