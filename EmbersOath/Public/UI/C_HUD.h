// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Game/C_Enums.h"
#include "C_HUD.generated.h"

class UC_UIManagerSubsystem;
class UC_UserWidget;
class UC_BaseWidgetController;
class UC_OverlayWidgetController;
class UC_OptionsWidgetController;
class UC_InGameWidgetController;
struct FWidgetControllerParams;
class APlayerController;
class UAbilitySystemComponent;
class UAttributeSet;

// forward declare custom classes
class AC_PlayerController;
class UC_OverlayWidgetController;
class UC_SpellMenuWidgetController;
class UC_GameInstance;
class AC_GameMode;
class UC_QuestWidgetController;

UCLASS()
class EMBERSOATH_API AC_HUD : public AHUD
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	bool IsMenuWidgetVisible(EUIWidgetId WidgetId) const;

	void PassQuestWidgetControllerFromIngameToOverlay(const FWidgetControllerParams& WCParams);
	
	UC_OverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);
	UC_SpellMenuWidgetController* GetSpellMenuWidgetController(const FWidgetControllerParams& WCParams);

	#pragma region Overlay Methods
		// Registers Overlay type and shows it = like initOverlay
		UFUNCTION(BlueprintCallable, Category = "UI")
		void ShowOverlay(AC_PlayerController* PC, UC_GameInstance* GI, AC_GameMode* GM, UAbilitySystemComponent* ASC, UAttributeSet* AS, int32 ZOrder = 0);

		UFUNCTION(BlueprintCallable, Category = "UI")
		void HideOverlay();

		UFUNCTION(BlueprintPure, Category = "UI")
		bool IsOverlayVisible() const;
	#pragma endregion

	#pragma region PauseMenu Methods
		UFUNCTION(BlueprintCallable, Category = "UI")
		void ShowPauseMenu(AC_PlayerController* PC, UC_GameInstance* GI, AC_GameMode* GM, int32 ZOrder = 0);

		UFUNCTION(BlueprintCallable, Category = "UI")
		void HidePauseMenu();

		UFUNCTION(BlueprintPure, Category = "UI")
		bool IsPauseMenuVisible() const;
	#pragma endregion

	#pragma region OptionsMenu Methods
		UFUNCTION(BlueprintCallable, Category = "UI")
		void ShowOptionsMenu(AC_PlayerController* PC, UC_GameInstance* GI, AC_GameMode* GM, int32 ZOrder = 0);
	
		UFUNCTION(BlueprintCallable, Category = "UI")
		void HideOptionsMenu();
	
		UFUNCTION(BlueprintPure, Category = "UI")
		bool IsOptionsMenuVisible() const;
	#pragma endregion

	#pragma region InGameMenu Methods
		UFUNCTION(BlueprintCallable, Category = "UI")
		void ShowInGameMenu(AC_PlayerController* PC, UC_GameInstance* GI, AC_GameMode* GM, UAbilitySystemComponent* ASC, UAttributeSet* AS, int32 ZOrder = 0);
	
		UFUNCTION(BlueprintCallable, Category = "UI")
		void HideInGameMenu();
	
		UFUNCTION(BlueprintPure, Category = "UI")
		bool IsInGameMenuVisible() const;
	#pragma endregion

private: //Functions
	void HideMenuWidget(EUIWidgetId WidgetId);
	bool ShowMenuWidget(EUIWidgetId WidgetId, TSubclassOf<UC_UserWidget> WidgetClass, TSubclassOf<UC_BaseWidgetController> WidgetControllerClass,
						AC_PlayerController* PC,
						UC_GameInstance* GI,
						AC_GameMode* GM,
						UAbilitySystemComponent* ASC = nullptr,
						UAttributeSet* AS = nullptr,
						int32 ZOrder = 0);

private: //Variables
	static const EUIWidgetId OverlayId;
	static const EUIWidgetId PauseMenuId;
	static const EUIWidgetId OptionsMenuId;
	static const EUIWidgetId InGameMenuId;

	TObjectPtr<UC_UIManagerSubsystem> UIManager;

	UPROPERTY()
	TObjectPtr<UC_UserWidget> OverlayWidget;

	UPROPERTY()
	TObjectPtr<UC_OverlayWidgetController> OverlayWidgetController;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UC_OverlayWidgetController> OverlayWidgetControllerClass;
	
	UPROPERTY()
	TObjectPtr<UC_UserWidget> TaskOverlayWidget;
	
	UPROPERTY()
	TObjectPtr<UC_SpellMenuWidgetController> SpellMenuWidgetController;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UC_SpellMenuWidgetController> SpellMenuWidgetControllerClass;
};