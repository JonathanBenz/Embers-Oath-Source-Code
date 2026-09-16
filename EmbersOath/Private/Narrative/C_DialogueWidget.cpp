// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "Narrative/C_DialogueWidget.h"
#include "Components/TextBlock.h"
#include "Narrative/C_DialogueInteraction.h"

void UC_DialogueWidget::UpdateTextBlocks(const FDialogueLine& DialogueLine)
{
	Speaker->SetText(DialogueLine.Speaker);
	Dialogue->SetText(DialogueLine.Text);
}