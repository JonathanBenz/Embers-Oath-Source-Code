// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "AI/C_InteractableNPC.h"
#include "Narrative/C_DialogueInteraction.h"
#include "Narrative/C_DialogueWidget.h"
#include "Components/SphereComponent.h"
#include "AI/C_SmoothFocusAIController.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/Player/Ember/C_Ember.h"

AC_InteractableNPC::AC_InteractableNPC()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerZone = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger Zone"));
	TriggerZone->SetupAttachment(GetMesh());
	TriggerZone->SetSphereRadius(200.0f);
}

void AC_InteractableNPC::BeginPlay()
{
	Super::BeginPlay();
	
	NPCAIController = Cast<AC_SmoothFocusAIController>(GetController());
	check(NPCAIController);

	DialoguePopupUI = CreateWidget<UC_DialogueWidget>(GetWorld(), DialoguePopupUIClass);
	check(DialoguePopupUI);
	
	NPCAIController->SmoothFocusInterpSpeed = 30.0f;
	bIsDialogueFinished = true;
}

/* Interface Functions */
void AC_InteractableNPC::Interact_Implementation(AC_Ember* Interactor) {}
bool AC_InteractableNPC::IsDialogueFinished_Implementation() { return bIsDialogueFinished; }
float AC_InteractableNPC::GetHoldDuration_Implementation() { return HoldDuration; }
FText AC_InteractableNPC::GetInteractionPrompt_Implementation() { return InteractionPrompt; }
EInteractionTypes AC_InteractableNPC::GetInteractionType_Implementation() { return InteractionType; }
/* End Interface Functions */

void AC_InteractableNPC::PlayDialogue(UC_DialogueInteraction* DialogueDataAsset)
{
	FacePlayer();
	DialoguePopupUI->AddToViewport();
	bIsDialogueFinished = false;
	CurrentDialogueIdx = 0;
	CurrentLines = DialogueDataAsset->Lines;
	PlayNextDialogueLine();
}

void AC_InteractableNPC::AdvanceDialogue_Implementation()
{
	if (bIsDialogueFinished) return;
	GetWorldTimerManager().ClearTimer(DialogueTimerHandle);
	PlayNextDialogueLine();
}

void AC_InteractableNPC::SkipDialogue_Implementation()
{
	GetWorldTimerManager().ClearTimer(DialogueTimerHandle);
	CurrentDialogueIdx = CurrentLines.Num();
	PlayNextDialogueLine();
}

void AC_InteractableNPC::PlayNextDialogueLine()
{
	// Base Case: Dialogue is finished!
	if (CurrentDialogueIdx >= CurrentLines.Num())
	{
		DialoguePopupUI->RemoveFromParent();
		ClearFocus();
		if (!bIsDialogueFinished) DialogueFinishedForFirstTime();
		DialogueFinished();
		bIsDialogueFinished = true;
		return;
	}

	DialoguePopupUI->UpdateTextBlocks(CurrentLines[CurrentDialogueIdx]);
	const float Delay = CurrentLines[CurrentDialogueIdx].Duration;
	CurrentDialogueIdx++;

	// Start next line of dialogue via recursion when duration of current line has passed.
	GetWorldTimerManager().SetTimer(DialogueTimerHandle, this, &AC_InteractableNPC::PlayNextDialogueLine, Delay, false);
}

void AC_InteractableNPC::FacePlayer() const { NPCAIController->SetFocus(UGameplayStatics::GetPlayerPawn(this, 0)); }
void AC_InteractableNPC::ClearFocus() const { NPCAIController->ClearFocus(EAIFocusPriority::Gameplay); }