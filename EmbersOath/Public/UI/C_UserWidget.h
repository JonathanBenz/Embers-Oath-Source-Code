// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_UserWidget.generated.h"

class UC_GameInstance;
class UC_BaseWidgetController;

UCLASS()

class EMBERSOATH_API UC_UserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UC_BaseWidgetController* InWidgetController);

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UC_BaseWidgetController> WidgetController;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DoWhenWidgetShown();

protected:
	/* When the Widget Controller is set for this particular Widget, use this blueprint event
	   to handle the Widget's logic and how it should handle the data from its widget controller. */
	UFUNCTION(BlueprintImplementableEvent)
	void OnWidgetControllerSet();
};