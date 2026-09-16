// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#include "UI/C_InGameWidgetController.h"
#include "UI/C_QuestWidgetController.h"
#include "UI/C_SpellMenuWidgetController.h"

void UC_InGameWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WCParams)
{
    Super::SetWidgetControllerParams(WCParams);

    //Forward the params to the child 
    if (QuestController) QuestController->SetWidgetControllerParams(WCParams);
    if (SpellMenuWidgetController) SpellMenuWidgetController->SetWidgetControllerParams(WCParams);
}

void UC_InGameWidgetController::EnsureSubControllersInitialized()
{
    InitializeSubControllers();
}

UC_SpellMenuWidgetController* UC_InGameWidgetController::GetSpellMenuWidgetController(const FWidgetControllerParams& WCParams)
{
    if (SpellMenuWidgetController == nullptr)
    {
        SpellMenuWidgetController = NewObject<UC_SpellMenuWidgetController>(this, SpellMenuWidgetControllerClass);  
        SpellMenuWidgetController->SetWidgetControllerParams(WCParams);
        SpellMenuWidgetController -> BindCallbacksToDependencies();
    }
    
    return SpellMenuWidgetController;
}

void UC_InGameWidgetController::InitializeSubControllers()
{
    if (!QuestController)
    {
        // UIManager gives the Widget the InGameController and that InGameController creates and manages the QuestController.
        QuestController = NewObject<UC_QuestWidgetController>(this);
    }
}

void UC_InGameWidgetController::BroadcastInitialValues()
{
	if (QuestController)
    {
        UE_LOG(LogTemp, Warning, TEXT("In-gameWIdgetController[%p]: BroadcastInitial Values"), this);
        QuestController->BroadcastInitialValues();
	}
}

void UC_InGameWidgetController::BindCallbacksToDependencies()
{
    // If we are already bound, do nothing.
    if (bIsBoundToDependencies) return;

	// Forward to child so we can listen for quest updates.
    if (QuestController)
    {
        QuestController->BindCallbacksToDependencies();
        bIsBoundToDependencies = true;
    }
}