// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Narrative/InteractableInterface.h"
#include "C_InteractableNPC.generated.h"

class UUserWidget;
class USphereComponent;
class UC_DialogueInteraction;
class UC_DialogueWidget;
class AC_SmoothFocusAIController;
class AC_Ember;
struct FDialogueLine;

UCLASS()
class EMBERSOATH_API AC_InteractableNPC : public ACharacter, public IInteractableInterface
{
	GENERATED_BODY()

public:
	AC_InteractableNPC();

	virtual void Interact_Implementation(AC_Ember* Interactor) override;
	virtual void AdvanceDialogue_Implementation() override;
	virtual void SkipDialogue_Implementation() override;
	virtual bool IsDialogueFinished_Implementation() override;
	virtual float GetHoldDuration_Implementation() override;
	virtual EInteractionTypes GetInteractionType_Implementation() override;
	virtual FText GetInteractionPrompt_Implementation() override;

	// This event only gets called when the Main Dialogue of a character is finished once. Does not care about Repeat Dialogue. 
	UFUNCTION(BlueprintImplementableEvent)
	void DialogueFinishedForFirstTime();

	// This event does not care if a Main or Repeat Dialogue is finished, unlike DialogueFinishedForFirstTime(). 
	UFUNCTION(BlueprintImplementableEvent)
	void DialogueFinished();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void PlayDialogue(UC_DialogueInteraction* DialogueDataAsset);

	void PlayNextDialogueLine();

	UFUNCTION(BlueprintCallable)
	void FacePlayer() const; 

	UFUNCTION(BlueprintCallable)
	void ClearFocus() const;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AC_SmoothFocusAIController> NPCAIController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UC_DialogueInteraction> MainDialogueInteraction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UC_DialogueInteraction> RepeatDialogueInteraction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UC_DialogueWidget> DialoguePopupUIClass;

	UPROPERTY()
	TObjectPtr<UC_DialogueWidget> DialoguePopupUI;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
	bool bIsRepeating = false;

	UPROPERTY(EditAnywhere, Category = "Dialogue")
	EInteractionTypes InteractionType = EInteractionTypes::EIT_Press;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
	float HoldDuration = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
	FText InteractionPrompt = NSLOCTEXT("Dialogue", "InteractionPrompt", "to talk");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (AllowPrivateAccess = "true"))
	bool bIsDialogueFinished = true;

	UPROPERTY(VisibleAnywhere, Category = "Dialogue")
	int32 CurrentDialogueIdx = 0;

	UPROPERTY(VisibleAnywhere, Category = "Dialogue")
	TArray<FDialogueLine> CurrentLines;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> TriggerZone;

	UPROPERTY()
	FTimerHandle DialogueTimerHandle;
};