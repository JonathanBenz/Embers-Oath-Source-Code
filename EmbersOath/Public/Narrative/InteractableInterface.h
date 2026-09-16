// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "C_NarrativeEnums.h"
#include "InteractableInterface.generated.h"

UINTERFACE(MinimalAPI, BlueprintType)
class UInteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class EMBERSOATH_API IInteractableInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Interact(class AC_Ember* Interactor);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void AdvanceDialogue();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SkipDialogue();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	EInteractionTypes GetInteractionType();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	float GetHoldDuration();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FText GetInteractionPrompt();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsDialogueFinished();
};