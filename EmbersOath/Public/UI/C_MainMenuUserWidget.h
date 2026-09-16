// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "UI/C_UserWidget.h"
#include "C_MainMenuUserWidget.generated.h"

class AC_HUD;
class AC_PlayerController;
class AC_GameMode;
class UC_GameInstance;
class UC_OptionsWidgetController;
class UC_UIManagerSubsystem;

UCLASS()
class EMBERSOATH_API UC_MainMenuUserWidget : public UC_UserWidget
{
	GENERATED_BODY()

private: //Functions
	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	bool InitializeImpVariables();

	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	void InitializeControllersAndSubsystems();

	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	void OpenOptionsMenu();

	UFUNCTION(BlueprintCallable, meta = (AllowPrivateAccess = "true"))
	void SetupMainMenuWidgetInOptionsController(UC_MainMenuUserWidget* MainMenuWidget);

private: //Variables
	UC_UIManagerSubsystem* UIManager;

	UPROPERTY(BlueprintReadOnly, Category = "ImpVariables", meta = (AllowPrivateAccess = "true"))
	UC_OptionsWidgetController* OptionsController;

	UPROPERTY(BlueprintReadOnly, Category="ImpVariables", meta = (AllowPrivateAccess = "true"))
	AC_PlayerController* PlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "ImpVariables", meta = (AllowPrivateAccess = "true"))
	AC_GameMode* GameMode;

	UPROPERTY(BlueprintReadOnly, Category = "ImpVariables", meta = (AllowPrivateAccess = "true"))
	UC_GameInstance* GameInstance;

	UPROPERTY(BlueprintReadOnly, Category = "ImpVariables", meta = (AllowPrivateAccess = "true"))
	AC_HUD* HUD;
};