// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#include "UI/C_UIManagerSubsystem.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"
#include "UI/C_UserWidget.h"
#include "UI/C_BaseWidgetController.h"

void UC_UIManagerSubsystem::RegisterWidgetType(EUIWidgetId WidgetId, TSubclassOf<UC_UserWidget> WidgetClass, TSubclassOf<UC_BaseWidgetController> ControllerClass, int32 DefaultZOrder)
{
	if (!WidgetClass || !ControllerClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UIManager: Invalid registration for widget enum %d."), static_cast<int32>(WidgetId));
		return;
	}
	FRegisteredWidgetInfo Info;
	Info.WidgetClass = WidgetClass;
	Info.ControllerClass = ControllerClass;
	Info.DefaultZOrder = DefaultZOrder;

	// Create controller instance early
	UObject* GameInstanceObj = GetGameInstance();
	Info.ControllerInstance = NewObject<UC_BaseWidgetController>(GameInstanceObj, ControllerClass);

	Info.ControllerInstance->InitializeSubControllers(); // Initialize any sub-controllers if needed

	Registered.Add(WidgetId, Info);
}

void UC_UIManagerSubsystem::RegisterWidgetWithExistingController(EUIWidgetId NewWidgetId, TSubclassOf<UC_UserWidget> WidgetClass, EUIWidgetId SourceWidgetId, int32 DefaultZOrder)
{
	if (const FRegisteredWidgetInfo* SourceInfo = Registered.Find(SourceWidgetId))
	{
		if (SourceInfo->ControllerInstance)
		{
			FRegisteredWidgetInfo NewInfo;
			NewInfo.WidgetClass = WidgetClass;
			// Copy the pointer from the source
			NewInfo.ControllerInstance = SourceInfo->ControllerInstance;
			NewInfo.ControllerClass = SourceInfo->ControllerClass;
			NewInfo.DefaultZOrder = DefaultZOrder;

			Registered.Add(NewWidgetId, NewInfo);
		}
	}
}

bool UC_UIManagerSubsystem::GetRegisteredWidgetInfo(EUIWidgetId WidgetId, FRegisteredWidgetInfo& OutInfo) const
{
	if (const FRegisteredWidgetInfo* Found = Registered.Find(WidgetId))
	{
		OutInfo = *Found;
		return true;
	}
	return false;
}

bool UC_UIManagerSubsystem::ShowWidget(EUIWidgetId WidgetId, const FWidgetControllerParams& Params, int32 ZOrderOverride)
{
	const FRegisteredWidgetInfo* Info = Registered.Find(WidgetId);
	if (!Info)
	{
		UE_LOG(LogTemp, Error, TEXT("UIManager: WidgetId %d not registered."), static_cast<int32>(WidgetId));
		return false;
	}

	// Get or create runtime entry - Using FindOrAdd so we can retrieve existing widget/controller if already created
	FWidgetRuntimeEntry& RuntimeEntry = Runtime.FindOrAdd(WidgetId);
	UC_UserWidget* Widget = RuntimeEntry.Widget.Get();
	UC_BaseWidgetController* Controller = Info->ControllerInstance.Get();

	// Create widget
	if (!Widget)
	{
		UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
		if (!World)
		{
			UE_LOG(LogTemp, Error, TEXT("UIManager: World is null."));
			return false;
		}

		Widget = CreateWidget<UC_UserWidget>(World, Info->WidgetClass);
		if (!Widget)
		{
			UE_LOG(LogTemp, Error, TEXT("UIManager: Failed to create widget '%s'."), *Info->WidgetClass->GetName());
			return false;
		}
		RuntimeEntry.Widget = Widget;
	}

	// Create controller 
	if (!Controller)
	{
		UObject* GameInstanceObj = GetGameInstance();
		Controller = NewObject<UC_BaseWidgetController>(GameInstanceObj, Info->ControllerClass);
		Controller->InitializeSubControllers(); // Initialize any sub-controllers if needed
		if (!Controller)
		{
			UE_LOG(LogTemp, Error, TEXT("UIManager: Failed to create controller '%s'."), *Info->ControllerClass->GetName());
			return false;
		}
	}

	// Set the controller parameters everytime we show the widget, in case there are changes in the backend data that the widget depends on
	//Ensure parameters are set before binding, and you must allow re-binding if the player changes
	Controller->SetWidgetControllerParams(Params);
	RuntimeEntry.Controller = Controller;

	const int32 ZOrder = (ZOrderOverride == INDEX_NONE) ? Info->DefaultZOrder : ZOrderOverride;

	if (!Widget->IsInViewport()) Widget->AddToViewport(ZOrder);

	Widget->SetVisibility(ESlateVisibility::Visible);

	RuntimeEntry.ZOrder = ZOrder;

	Controller->BindCallbacksToDependencies();

	// Wire widget <-> controller
	Widget->SetWidgetController(Controller);
	Controller->BroadcastInitialValues();

	Widget->DoWhenWidgetShown(); // To set user focus when widget is shown
	return true;
}

void UC_UIManagerSubsystem::HideWidget(EUIWidgetId WidgetId)
{
	if (FWidgetRuntimeEntry* Entry = Runtime.Find(WidgetId))
	{
		if (UC_UserWidget* Widget = Entry->Widget.Get())
			Widget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

bool UC_UIManagerSubsystem::IsWidgetVisible(EUIWidgetId WidgetId) const
{
	if (const FWidgetRuntimeEntry* Entry = Runtime.Find(WidgetId))
	{
		if (const UC_UserWidget* Widget = Entry->Widget.Get())
			return Widget->GetVisibility() == ESlateVisibility::Visible;
	}
	return false;
}

UC_UserWidget* UC_UIManagerSubsystem::GetWidget(EUIWidgetId WidgetId) const
{
	if (const FWidgetRuntimeEntry* Entry = Runtime.Find(WidgetId))
		return Entry->Widget.Get();
	
	return nullptr;
}

void UC_UIManagerSubsystem::TeardownWidget(EUIWidgetId WidgetId)
{
	//Be careful when calling this for shared controllers, as it will destroy the controller instance as well.
	if (FWidgetRuntimeEntry* Entry = Runtime.Find(WidgetId))
	{
		if (UC_BaseWidgetController* Controller = Entry->Controller.Get())
		{
			// Optional: Controller->UnbindCallbacksFromDependencies();
			Controller->ConditionalBeginDestroy();
		}
		if (UC_UserWidget* Widget = Entry->Widget.Get())
		{
			Widget->RemoveFromParent();
			Widget->ConditionalBeginDestroy();
		}
		Runtime.Remove(WidgetId);
	}
}