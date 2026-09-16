// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "C_SpellMenuWidgetController.h"
#include "UI/C_BaseWidgetController.h"
#include "Game/C_Enums.h"
#include "Game/C_Structs.h"
#include "C_InGameWidgetController.generated.h"

class UC_QuestWidgetController;
class UC_SpellMenuWidgetController;

UCLASS(BlueprintType, Blueprintable)
class EMBERSOATH_API UC_InGameWidgetController : public UC_BaseWidgetController
{
	GENERATED_BODY()

public:
	// Override to set the widget controller parameters for sub widgets as well
	virtual void SetWidgetControllerParams(const FWidgetControllerParams& WCParams) override;
	virtual void InitializeSubControllers() override;

	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	// lets HUD force initialization even if In-Game menu is never opened
	UFUNCTION(BlueprintCallable, Category="WidgetController")
	void EnsureSubControllersInitialized();
	
	UC_SpellMenuWidgetController* GetSpellMenuWidgetController(const FWidgetControllerParams& WCParams);

public: // Variables

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UC_QuestWidgetController> QuestController;
	
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UC_SpellMenuWidgetController> SpellMenuWidgetController;
	
	UPROPERTY(EditAnywhere, Category="WidgetController")
	TSubclassOf<UC_SpellMenuWidgetController> SpellMenuWidgetControllerClass;
};