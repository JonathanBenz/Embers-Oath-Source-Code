// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Game/C_Enums.h"
#include "C_UIManagerSubsystem.generated.h"

class UC_BaseWidgetController;
class UC_UserWidget;
struct FWidgetControllerParams;

/// Structure to hold information about registered widget types.
/// Class descriptors not object instances.
USTRUCT(BlueprintType)
struct FRegisteredWidgetInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UC_UserWidget> WidgetClass = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UC_BaseWidgetController> ControllerClass = nullptr;

	UPROPERTY()
	TObjectPtr<UC_BaseWidgetController> ControllerInstance = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 DefaultZOrder = 0;
};

USTRUCT()
struct FWidgetRuntimeEntry
{
	GENERATED_BODY()

	TWeakObjectPtr<UC_UserWidget> Widget;
	TWeakObjectPtr<UC_BaseWidgetController> Controller;
	int32 ZOrder = 0;
};

UCLASS(BlueprintType, Blueprintable)
class EMBERSOATH_API UC_UIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/// Register and create the particular derived controller class, but store the reference as the base controller type.
	UFUNCTION(BlueprintCallable)
	void RegisterWidgetType(EUIWidgetId WidgetId, TSubclassOf<UC_UserWidget> WidgetClass, TSubclassOf<UC_BaseWidgetController> ControllerClass, int32 DefaultZOrder = 0);

	// Register a widget ID using an existing controller instance from another ID
	UFUNCTION(BlueprintCallable)
	void RegisterWidgetWithExistingController(EUIWidgetId NewWidgetId, TSubclassOf<UC_UserWidget> WidgetClass, EUIWidgetId SourceWidgetId, int32 DefaultZOrder = 0);

	UFUNCTION(BlueprintPure)
	bool GetRegisteredWidgetInfo(EUIWidgetId WidgetId, FRegisteredWidgetInfo& OutInfo) const;

	UFUNCTION(BlueprintCallable)
	bool ShowWidget(EUIWidgetId WidgetId, const FWidgetControllerParams& Params, int32 ZOrderOverride = 0);

	UFUNCTION(BlueprintCallable)
	void HideWidget(EUIWidgetId WidgetId);

	UFUNCTION(BlueprintPure)
	bool IsWidgetVisible(EUIWidgetId WidgetId) const;

	UFUNCTION(BlueprintPure)
	UC_UserWidget* GetWidget(EUIWidgetId WidgetId) const;

	UFUNCTION(BlueprintCallable)
	void TeardownWidget(EUIWidgetId WidgetId);

	template<typename TController>
	TController* GetRegisteredControllerTyped(EUIWidgetId WidgetId) const
	{
		if (const FRegisteredWidgetInfo* RegisteredInfo = Registered.Find(WidgetId))
		{
			return Cast<TController>(RegisteredInfo->ControllerInstance);
		}
		return nullptr;
	}

private:
	UPROPERTY()
	TMap<EUIWidgetId, FRegisteredWidgetInfo> Registered;

	UPROPERTY()
	TMap<EUIWidgetId, FWidgetRuntimeEntry> Runtime;
};