// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Hritish Duvvur, Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "Game/C_Enums.h"
#include "Game/C_Structs.h"
#include "C_PlayerController.generated.h"

class AC_MagicCircle;
class UInputMappingContext;
class UInputAction;
class UC_GameInstance;
class AC_GameMode;
class AC_HUD;
class UC_QuestManagerComponent;
class UDA_EmberInputConfig;
class UASC_AbilitySystemComponent;
class UAudioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputTypeChanged, EInputTypes, CurrentInputType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInGameMenuOpened);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaypointActivated, FWaypointDataForMap, WaypointInformation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaypointHidden, FWaypointDataForMap, WaypointInformation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerLeveledUp, float, XPIncreased, bool, bShouldAddSpellPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePausedStateChange, bool, bIsGamePaused);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FToShowSideOnboardingPanel, EPlayerOnboardingPhases, OnboardingPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDemoEnded);

UCLASS()
class EMBERSOATH_API AC_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// Events for input type changes
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "InputType")
	FOnInputTypeChanged OnInputTypeChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Waypoints")
	FOnWaypointActivated OnWaypointActivated;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Waypoints")
	FOnWaypointHidden OnWaypointHidden;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "PlayerLevelUP")
	FOnPlayerLeveledUp OnPlayerLevelUp;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "PlayerLevelUP")
	FOnGamePausedStateChange OnGamePausedStateChanged;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "OnboardingEvents")
	FOnInGameMenuOpened OnInGameMenuOpened;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "OnboardingEvents")
	FToShowSideOnboardingPanel ToShowSideOnboardingPanel;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "Demo")
	FOnDemoEnded OnDemoEnded;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* UIIMC;

	// Cache all active waypoints as Map can open after they are set Active, so we need to store their values
	UPROPERTY()
	TArray<FWaypointDataForMap> CachedWaypointInformations;

	UPROPERTY()
	TArray<FWaypointDataForMap> CachedHiddenWaypointInformations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Onboarding")
	EUIOnboardingPhases CurrentUIOnboardingPhase = EUIOnboardingPhases::NotStarted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Onboarding")
	EPlayerOnboardingPhases CurrentPlayerOnboardingPhase = EPlayerOnboardingPhases::NotStarted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Onboarding")
	bool bSkipOnboarding = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Onboarding")
	bool bShownUsingSpells = false;

public: //Functions
	AC_PlayerController();
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	void LoadQuestStateFromGameInstance();

	UFUNCTION(BlueprintCallable, Category = "UI|Pause")
	void HandlePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI|Pause")
	void HandleInGameMenu();

	UFUNCTION(BlueprintCallable, Category = "InteractionFunctions")
	void EnableCutsceneIMC();

	UFUNCTION(BlueprintCallable, Category = "InteractionFunctions")
	void DisableCutsceneIMC();

	#pragma region **IMC Tracking**
	// Register new IMC's added in game for tracking purposes
	void RegisterAndAddIMC(UInputMappingContext* NewContext, int32 Priority);

	// Overloaded: Swap one IMC for another
	void RegisterAndAddIMC(UInputMappingContext* NewContext, UInputMappingContext* OldContext, int32 Priority);

	// Call this instead of Subsystem->RemoveMappingContext
	UFUNCTION(BlueprintCallable, Category = "Input")
	void UnregisterAndRemoveIMC(UInputMappingContext* ContextToRemove);

	UFUNCTION(BlueprintCallable, Category = "Input")
	void AddMenuIMC();
	#pragma endregion

	UFUNCTION(BlueprintCallable, Category = "UI|Pause")
	void PauseGame();

	UFUNCTION(BlueprintCallable, Category = "UI|Pause")
	void ResumeGame();
	
	//Magic Circle Functions
	bool IsAimingMagicCircle();
	UFUNCTION(BlueprintCallable)
	void ShowMagicCircle(UMaterialInterface* DecalMaterial = nullptr);
	
	UFUNCTION(BlueprintCallable)
	FVector GetMagicCircleLocation();
	
	UFUNCTION(BlueprintCallable)
	void HideMagicCircle();
	
	void AimMagicCircle(const FInputActionValue& Value);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Checkpoint System")
	void OnRespawnAfterDeath();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Checkpoint System")
	void OnGameRestarted();

	UFUNCTION(BlueprintCallable)
	void DoOnWaypointActivated(FWaypointDataForMap WaypointInformation);
		
	UFUNCTION(BlueprintCallable)
	void DoOnWaypointHidden(FWaypointDataForMap WaypointInformation);

	UFUNCTION(BlueprintCallable)
	void BroadcastCachedActiveWaypointInformation();

	UFUNCTION(BlueprintCallable)
	void BroadcastCachedHiddenWaypointInformation();

	UFUNCTION(BlueprintCallable)
	void SwitchCurrentUIOnboardingPhase(EUIOnboardingPhases NewPhase);

	UFUNCTION(BlueprintCallable)
	void SwitchCurrentPlayerOnboardingPhase(EPlayerOnboardingPhases NewPhase);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool HasCompletedEitherUIOnboarding() const
	{
		return CurrentUIOnboardingPhase == EUIOnboardingPhases::CompletedMap ||
			   CurrentUIOnboardingPhase == EUIOnboardingPhases::CompletedQuest ||
			   CurrentUIOnboardingPhase == EUIOnboardingPhases::Completed;
	}

	UFUNCTION(BlueprintCallable)
	void EnableDisableInputActionOnPause(UInputAction* InputAction, bool bShouldEnable);

	/* MUSIC PLAYING */
	UFUNCTION(BlueprintCallable)
	void PlayIdleMusic();

	UFUNCTION(BlueprintCallable)
	void StopIdleMusic();

	UFUNCTION(BlueprintCallable)
	void PlayCombatMusic();

	UFUNCTION(BlueprintCallable)
	void StopCombatMusic();
	/* END MUSIC PLAYING */

	// Get the value of CameraYawSensitivity.
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCameraYawSensitivity() { return CameraYawSensitivity; }

	// Get the value of CameraPitchSensitivity.
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCameraPitchSensitivity() { return CameraPitchSensitivity; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetCameraYawSensitivity(float SensitivityValue) { CameraYawSensitivity = SensitivityValue; }

	// Change the value of CameraPitchSensitivity. Default value = 1.0f;
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetCameraPitchSensitivity(float SensitivityValue) { CameraPitchSensitivity = SensitivityValue; }

protected: //Variables
	UPROPERTY(BlueprintReadOnly, Category = "IMPGameVariables")
	TObjectPtr<UC_GameInstance> GameInstance;

	UPROPERTY(BlueprintReadOnly, Category = "IMPGameVariables")
	TObjectPtr<AC_GameMode> GameMode;

	UPROPERTY(BlueprintReadOnly, Category = "IMPGameVariables")
	TObjectPtr<AC_HUD> HUD;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Music")
	TObjectPtr<UAudioComponent> IdleMusic;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Music")
	TObjectPtr<UAudioComponent> CombatMusic;
	
	//Magic Circle Variables
	UPROPERTY(EditAnywhere, Category = "Spells")
	float MagicCircleMoveSpeed = 800.0f;

protected: //Functions
	UFUNCTION(BlueprintImplementableEvent)
	void SetBPEmberVariable();

	UFUNCTION(BlueprintImplementableEvent)
	void SetBPPhoenixVariable();

private:
	UC_QuestManagerComponent* QuestManager = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* MenuIMC;

	// Interaction IMC
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))	
	UInputMappingContext* CutsceneIMC;

	// List of all the game's IMC's for tracking purposes
	UPROPERTY()
	TMap<TObjectPtr<UInputMappingContext>, int32> ActiveContexts;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> PauseMenuAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InGameMenuAction;
	
	// Input Action for GAS Abilities
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDA_EmberInputConfig> InputConfig;
	
	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);
	
	UPROPERTY()
	TObjectPtr<UASC_AbilitySystemComponent> EmberAbilitySystemComponent;
	
	UASC_AbilitySystemComponent* GetASC();
	
	// Magic Circle Variables
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AC_MagicCircle> MagicCircleClass;
	
	UPROPERTY()
	TObjectPtr<AC_MagicCircle> MagicCircle;
	
	float CurrentCircleDistance = 500.f;
	float MinDistance = 200.f;
	float MaxDistance = 1000.f;
	float MoveSpeed = 50.f;
	
	FHitResult CursorHit;
	
	//Magic Circle Functions
	void CursorTrace();
	void UpdateMagicCircleLocation();
	
	// This stores the raw input from the stick
	FVector2D MagicCircleInput;

	void DisableSlateDPadNavigation();
	void EnableSlateDPadNavigation();

	UPROPERTY(EditAnywhere, Category = "Music")
	float MusicFadeOutDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility Settings", meta = (AllowPrivateAccess = "true"))
	float CameraYawSensitivity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Accessibility Settings", meta = (AllowPrivateAccess = "true"))
	float CameraPitchSensitivity = 1.0f;
};