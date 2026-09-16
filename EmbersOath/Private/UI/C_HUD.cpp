// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#include "UI/C_HUD.h"
#include "UI/C_UIManagerSubsystem.h"
#include "Game/C_GameInstance.h" 
#include "UI/C_UserWidget.h"
#include "UI/C_BaseWidgetController.h"
#include "UI/C_OverlayWidgetController.h"
#include "UI/C_QuestWidgetController.h"
#include "UI/C_OptionsWidgetController.h"
#include "UI/C_InGameWidgetController.h"

const EUIWidgetId AC_HUD::OverlayId = EUIWidgetId::Overlay;
const EUIWidgetId AC_HUD::PauseMenuId = EUIWidgetId::PauseMenu;
const EUIWidgetId AC_HUD::OptionsMenuId = EUIWidgetId::Options;
const EUIWidgetId AC_HUD::InGameMenuId = EUIWidgetId::InGameMenu;

void AC_HUD::BeginPlay()
{
	Super::BeginPlay();
	UGameInstance* BaseGI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
	if (!BaseGI) return;

	UIManager = BaseGI->GetSubsystem<UC_UIManagerSubsystem>();
	if (!UIManager)
	{
		UE_LOG(LogTemp, Error, TEXT("HUD: UIManagerSubsystem not found on GameInstance during BeginPlay."));
		return;
	}
}

#pragma region ShowSpecificMenu
/// <summary>
/// When this is called, the respective menu will initialize along with the Widget Controller. It will then display to the viewport. 
/// It registers the widget type with the UIManagerSubsystem if it hasn't already been registered.
/// It also adds the widget to the runtime variable in the UIManagerSubsystem, when it is added to the viewport.
/// </summary>
/// <returns> Whether if the widget was successfully added to viewport or not</returns>
bool AC_HUD::ShowMenuWidget(EUIWidgetId WidgetId, TSubclassOf<UC_UserWidget> WidgetClass, TSubclassOf<UC_BaseWidgetController> WidgetControllerClass, AC_PlayerController* PC, UC_GameInstance* GI, AC_GameMode* GM, UAbilitySystemComponent* ASC, UAttributeSet* AS, int32 ZOrder)
{
	if (!PC || !GI || !GM )
	{
		UE_LOG(LogTemp, Error, TEXT("HUD: ShowMenuWidget missing required parameters (PC/GI/GM)."));
		return false;
	}

	if (!UIManager)
	{
		UE_LOG(LogTemp, Error, TEXT("HUD: UIManagerSubsystem not found on GameInstance."));
		return false;
	}

	// If the widget type is already registered, verify classes match; otherwise register it.
	FRegisteredWidgetInfo ExistingInfo;
	if (UIManager->GetRegisteredWidgetInfo(WidgetId, ExistingInfo))
	{
		// If the incoming classes differ from the registered ones, warn (controllers are set at game instance start)
		if (ExistingInfo.WidgetClass != WidgetClass || ExistingInfo.ControllerClass != WidgetControllerClass)
			UE_LOG(LogTemp, Warning, TEXT("HUD: ShowMenuWidget called with different WidgetClass/ControllerClass than previously registered for WidgetId %d."), static_cast<int32>(WidgetId));
	}
	else
	{
		// Not registered yet -> register now (safe to call multiple times elsewhere but avoid overwriting existing configs)
		UIManager->RegisterWidgetType(WidgetId, WidgetClass, WidgetControllerClass, ZOrder);
	}

	const FWidgetControllerParams WidgetControllerParams(PC, GI, GM, ASC, AS);
	if (UIManager->ShowWidget(WidgetId, WidgetControllerParams, ZOrder)) 
		return true;

	return false;
}

void AC_HUD::ShowOverlay(AC_PlayerController* PC, UC_GameInstance* GI, AC_GameMode* GM, UAbilitySystemComponent* ASC, UAttributeSet* AS, int32 ZOrder)
{
	ShowMenuWidget(OverlayId, GI->OverlayWidgetClass, GI->OverlayWidgetControllerClass, PC, GI, GM, ASC, AS, ZOrder);

	const FWidgetControllerParams WidgetControllerParams(PC, GI, GM, ASC, AS);
	PassQuestWidgetControllerFromIngameToOverlay(WidgetControllerParams);
}

void AC_HUD::ShowPauseMenu(AC_PlayerController* PC, UC_GameInstance* GI, AC_GameMode* GM, int32 ZOrder)
{
	ShowMenuWidget(PauseMenuId, GI->PauseMenuWidgetClass, GI->OptionsMenuWidgetControllerClass, PC, GI, GM, nullptr, nullptr, ZOrder);
}

void AC_HUD::ShowOptionsMenu(AC_PlayerController* PC, UC_GameInstance* GI, AC_GameMode* GM, int32 ZOrder)
{
	// Passing in the OptionsMenu share the same controller class as the PauseMenu
	ShowMenuWidget(OptionsMenuId, GI->OptionsMenuWidgetClass, GI->OptionsMenuWidgetControllerClass, PC, GI, GM, nullptr, nullptr, ZOrder);
}

void AC_HUD::ShowInGameMenu(AC_PlayerController* PC, UC_GameInstance* GI, AC_GameMode* GM, UAbilitySystemComponent* ASC, UAttributeSet* AS, int32 ZOrder)
{
	ShowMenuWidget(InGameMenuId, GI->InGameMenuWidgetClass, GI->InGameMenuWidgetControllerClass, PC, GI, GM, ASC, AS, ZOrder);
}
#pragma endregion

#pragma region HideSpecificMenu
/// <summary>
/// Method to hide the specified menu widget. This only removes it from the viewport, it does not destroy it.
/// </summary>
void AC_HUD::HideMenuWidget(EUIWidgetId WidgetId)
{
	if (UIManager) UIManager->HideWidget(WidgetId);
}

void AC_HUD::HideOverlay()
{
	HideMenuWidget(OverlayId);
}

void AC_HUD::HidePauseMenu()
{
	HideMenuWidget(PauseMenuId);
}

void AC_HUD::HideOptionsMenu()
{
	// Options menu uses the excat same controller instance as the pause menu
	HideMenuWidget(OptionsMenuId);
}

void AC_HUD::HideInGameMenu()
{
	HideMenuWidget(InGameMenuId);
}
#pragma endregion

#pragma region MenuVisibility
/// <summary>
/// Returns true if the specified menu widget is currently visible.
/// IT is a query, not actions, thus read only const is used
/// </summary>
bool AC_HUD::IsMenuWidgetVisible(EUIWidgetId WidgetId) const
{
	if (UIManager)
		return UIManager->IsWidgetVisible(WidgetId);
	
	return false;
}

bool AC_HUD::IsOverlayVisible() const
{
	return IsMenuWidgetVisible(OverlayId);
}

bool AC_HUD::IsPauseMenuVisible() const
{
	return IsMenuWidgetVisible(PauseMenuId);
}

bool AC_HUD::IsOptionsMenuVisible() const
{
	return IsMenuWidgetVisible(OptionsMenuId);
}

bool AC_HUD::IsInGameMenuVisible() const
{
	return IsMenuWidgetVisible(InGameMenuId);
}
#pragma endregion


void AC_HUD::PassQuestWidgetControllerFromIngameToOverlay(const FWidgetControllerParams& WCParams)
{
	if (!UIManager) return;

	FRegisteredWidgetInfo OverlayInfo;
	if (!UIManager->GetRegisteredWidgetInfo(OverlayId, OverlayInfo)) return;

	UC_OverlayWidgetController* OverlayController = Cast<UC_OverlayWidgetController>(OverlayInfo.ControllerInstance);
	if (!OverlayController) return;

	FRegisteredWidgetInfo InGameInfo;
	if (!UIManager->GetRegisteredWidgetInfo(InGameMenuId, InGameInfo)) return;

	UC_InGameWidgetController* InGameController = Cast<UC_InGameWidgetController>(InGameInfo.ControllerInstance);
	if (!InGameController) return;

	// Make sure sub controllers exist
	InGameController->EnsureSubControllersInitialized();
	InGameController->SetWidgetControllerParams(WCParams);
	OverlayController->SetQuestControllerForOverlay(InGameController->QuestController);
}

UC_OverlayWidgetController* AC_HUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UC_OverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
		return OverlayWidgetController;
	}
	return OverlayWidgetController;
}

UC_SpellMenuWidgetController* AC_HUD::GetSpellMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	if (SpellMenuWidgetController == nullptr)
	{
		SpellMenuWidgetController = NewObject<UC_SpellMenuWidgetController>(this, SpellMenuWidgetControllerClass);
		SpellMenuWidgetController->SetWidgetControllerParams(WCParams);
		SpellMenuWidgetController->BindCallbacksToDependencies();
		SpellMenuWidgetController->BroadcastInitialValues();
		return SpellMenuWidgetController;
	}
	return SpellMenuWidgetController;
}