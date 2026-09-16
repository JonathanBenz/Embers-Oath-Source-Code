// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur

#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BFL_AbilitySystemLibrary.generated.h"

class UDA_AbilityInfo;
class UC_SpellMenuWidgetController;
class UC_OverlayWidgetController;
struct FWidgetControllerParams;

UCLASS()
class EMBERSOATH_API UBFL_AbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category="AbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, AC_HUD*& OutEmberHUD);
	
	UFUNCTION(BlueprintPure, Category="AbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UC_OverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintPure, Category="AbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UC_SpellMenuWidgetController* GetSpellMenuWidgetController(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintCallable, Category= "AbilitySystemLibrary|AbilityInfo", meta = (DefaultToSelf = "WorldContextObject"))
	static UDA_AbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);
};