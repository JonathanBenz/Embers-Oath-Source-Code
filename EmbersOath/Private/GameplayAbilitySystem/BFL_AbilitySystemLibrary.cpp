// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur

#include "GameplayAbilitySystem/BFL_AbilitySystemLibrary.h"
#include "UI/C_BaseWidgetController.h"
#include "Game/C_PlayerController.h"
#include "Game/C_GameInstance.h"
#include "AbilitySystemComponent.h"
#include "Characters/PS_PlayerState.h"
#include "Game/C_GameMode.h"
#include "Kismet/GameplayStatics.h"
#include "UI/C_HUD.h"

bool UBFL_AbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, AC_HUD*& OutEmberHUD)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		OutEmberHUD = Cast<AC_HUD>(PC->GetHUD());

		if (OutEmberHUD)
		{
			APS_PlayerState* PS = PC->GetPlayerState<APS_PlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			AC_PlayerController* CustomPC = Cast<AC_PlayerController>(PC);
			UC_GameInstance* GI = Cast<UC_GameInstance>(UGameplayStatics::GetGameInstance(WorldContextObject));
			
			OutWCParams.AttributeSet = AS;
			OutWCParams.AbilitySystemComponent = ASC;
			OutWCParams.PlayerController = CustomPC;
			OutWCParams.PlayerState = PS;
			OutWCParams.GameInstance = GI;
			return true;
		}

	}

	return false;
}

UC_OverlayWidgetController* UBFL_AbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	AC_HUD* EmberHUD = nullptr;
	
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, EmberHUD))
		return EmberHUD->GetOverlayWidgetController(WCParams);

	return nullptr;
}

UC_SpellMenuWidgetController* UBFL_AbilitySystemLibrary::GetSpellMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	AC_HUD* EmberHUD = nullptr;
	
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, EmberHUD))
		return EmberHUD->GetSpellMenuWidgetController(WCParams);
	
	return nullptr;
}

UDA_AbilityInfo* UBFL_AbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	const AC_GameMode* GM = Cast<AC_GameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (GM) return GM->AbilityInfo;

	return nullptr;
}