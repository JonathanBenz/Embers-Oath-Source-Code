// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#include "UI/C_MainMenuUserWidget.h"
#include "Game/C_PlayerController.h"
#include "Game/C_GameMode.h"
#include "Game/C_GameInstance.h"
#include "UI/C_HUD.h"
#include "Game/C_Enums.h"
#include "Kismet/GameplayStatics.h"
#include "UI/C_OptionsWidgetController.h"
#include "UI/C_UIManagerSubsystem.h"

bool UC_MainMenuUserWidget::InitializeImpVariables()
{
	PlayerController = Cast<AC_PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	GameMode = Cast<AC_GameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	GameInstance = Cast<UC_GameInstance>(GetWorld()->GetGameInstance());
	HUD = Cast<AC_HUD>(PlayerController->GetHUD());

	if (PlayerController && GameInstance && GameMode && HUD)
	{
		return true;
	}

	return false;
}

void UC_MainMenuUserWidget::InitializeControllersAndSubsystems()
{
	if (!GameInstance) return;

	// Get the Subsystem
	UIManager = GameInstance->GetSubsystem<UC_UIManagerSubsystem>();

	// Get the controller instance registered to the Options ID
	OptionsController = UIManager->GetRegisteredControllerTyped<UC_OptionsWidgetController>(EUIWidgetId::Options);
}

void UC_MainMenuUserWidget::OpenOptionsMenu()
{
	//Access the default object of the OptionsMenuWidgetControllerClass and set WidgetOpenedFrom there
	// Get the Subsystem
	if (UIManager && OptionsController)
	{
		//Set the property on the actual instance
		OptionsController->WidgetOpenedFrom = EUIWidgetId::MainMenu;

		//If the MainMenuWidget is valid, remove it from parent (to hide it) before opening Options Menu
		if (OptionsController->MainMenuWidget)
		{
			OptionsController->MainMenuWidget->RemoveFromParent();
		}
	}

	HUD->ShowOptionsMenu(PlayerController, GameInstance, GameMode, 1);
	OptionsController->GetPlayerController()->PauseGame();

}

void UC_MainMenuUserWidget::SetupMainMenuWidgetInOptionsController(UC_MainMenuUserWidget* MainMenuWidget)
{
	if (OptionsController)
	{
		OptionsController->MainMenuWidget = MainMenuWidget;
	}
}