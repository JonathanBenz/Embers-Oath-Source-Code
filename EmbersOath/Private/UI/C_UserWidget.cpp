// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "UI/C_UserWidget.h"
#include "UI/C_BaseWidgetController.h"
#include "Game/C_GameInstance.h"
#include <Kismet/GameplayStatics.h>

/// <summary>
/// Public function that can be called by another class (such as the HUD class or an Overlay class) to set a reference to a passed in WidgetController for this particular Widget. 
/// It also sets off the OnWidgetControllerSet() blueprint event, where the UI Programmer can then set up the remaining UI logic in blueprints
/// and be able to use whatever necessary data they need from the passed in WidgetController. 
/// </summary>
/// <param name="InWidgetController"> The Widget Controller that this Widget will use. The Widget Controller will be responsible for handling the data that the UI is responsible for. </param>
void UC_UserWidget::SetWidgetController(UC_BaseWidgetController* InWidgetController)
{
	WidgetController = InWidgetController;
	OnWidgetControllerSet();
}