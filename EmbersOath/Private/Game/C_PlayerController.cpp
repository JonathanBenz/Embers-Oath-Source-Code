// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Hritish Duvvur, Ayushi Parikh

#include "Game/C_PlayerController.h"
#include "Game/C_GameInstance.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystemInterface.h"
#include "Game/C_GameMode.h"
#include "UI/C_HUD.h"
#include "Quests/C_QuestManagerComponent.h"
#include "Game/C_Structs.h"
#include "UI/C_BaseWidgetController.h"
#include "Quests/I_QuestInterface.h"
#include "Characters/Player/Ember/C_Ember.h"
#include "Characters/Player/Phoenix/C_Phoenix.h"
#include <Kismet/GameplayStatics.h>
#include "UI/C_UserWidget.h"
#include "Containers/Map.h"
#include "GameplayTagContainer.h"
#include "Characters/IC_EmberInputComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GameFramework/PlayerInput.h"
#include "GameplayAbilitySystem/ASC_AbilitySystemComponent.h"
#include "Quests/C_QuestManagerSubsystem.h"
#include "GameplayAbilitySystem/BFL_AbilitySystemLibrary.h"
#include "InputMappingContext.h" 
#include "Game/C_HelperFunctionLibrary.h"
#include "Components/DecalComponent.h"
#include "Items/C_MagicCircle.h"
#include "Game/C_CheatManager.h"
#include "Framework/Application/NavigationConfig.h"
#include "Components/AudioComponent.h"
#include "UI/C_UIManagerSubsystem.h"
#include "UI/C_OptionsWidgetController.h"

AC_PlayerController::AC_PlayerController()
{
    CheatClass = UC_CheatManager::StaticClass();

    // Attach music audio components
    IdleMusic = CreateDefaultSubobject<UAudioComponent>(TEXT("Idle Music Player"));
    IdleMusic->SetupAttachment(GetRootComponent());
    IdleMusic->bAutoActivate = true;

    CombatMusic = CreateDefaultSubobject<UAudioComponent>(TEXT("Combat Music Player"));
    CombatMusic->SetupAttachment(GetRootComponent());
    CombatMusic->bAutoActivate = false;
}

void AC_PlayerController::BeginPlay()
{
    Super::BeginPlay();

	//RegisterAndAddIMC(MenuIMC, 4);
    GameInstance = Cast<UC_GameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
    GameMode = Cast<AC_GameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    
	// Only add menu IMC if we're not in the main menu
    if (GetWorld()->GetName() != UC_HelperFunctionLibrary::GetLevelNameFromEnum(ELevelNames::MainMenu).ToString())
		AddMenuIMC();
    
    // Get HUD after restoring quest state so UI can properly initialize
    HUD = Cast<AC_HUD>(GetHUD());

    //After HUD and quest state are ready, force UI to sync
    if (GameMode)
    {
        SetBPEmberVariable();
        SetBPPhoenixVariable();

        if (HUD && GameInstance)
        {
            const FWidgetControllerParams WCParams(this, GameInstance, GameMode, nullptr, nullptr);
            HUD->PassQuestWidgetControllerFromIngameToOverlay(WCParams);
        }
    }

    // Initial sync + bind to Options Sensitivity controls changes
    if (UC_UIManagerSubsystem* UIManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_UIManagerSubsystem>())
    {
        if (UC_OptionsWidgetController* OptionsCtrl = UIManager->GetRegisteredControllerTyped<UC_OptionsWidgetController>(EUIWidgetId::Options))
        {
            // Bind sensitivity values from Options Controller
            OptionsCtrl->OnCameraPitchSensitivityChanged.AddDynamic(this, &AC_PlayerController::SetCameraPitchSensitivity);
            OptionsCtrl->OnCameraYawSensitivityChanged.AddDynamic(this, &AC_PlayerController::SetCameraYawSensitivity);
        }
    }

    LoadQuestStateFromGameInstance();
}

void AC_PlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    
    CursorTrace();
    UpdateMagicCircleLocation();
}

void AC_PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    if (UIC_EmberInputComponent* EmberInputComponent = CastChecked<UIC_EmberInputComponent>(InputComponent))
    {
        EmberInputComponent->BindAbilityActions(InputConfig, this, &AC_PlayerController::AbilityInputTagPressed, &AC_PlayerController::AbilityInputTagReleased, &AC_PlayerController::AbilityInputTagHeld);
        EmberInputComponent->BindAction(PauseMenuAction, ETriggerEvent::Completed, this, &AC_PlayerController::HandlePauseMenu);
        EmberInputComponent->BindAction(InGameMenuAction, ETriggerEvent::Completed, this, &AC_PlayerController::HandleInGameMenu);
    }
}

void AC_PlayerController::PauseGame()
{
	GameInstance->bIsGamePaused = true;
    UGameplayStatics::SetGamePaused(GetWorld(), true);
	OnGamePausedStateChanged.Broadcast(true);

    // Switch input mode
    bShowMouseCursor = true;
    SetInputMode(FInputModeGameAndUI());
    
    RegisterAndAddIMC(UIIMC, 10);
}

void AC_PlayerController::ResumeGame()
{
    if(GameInstance)
	    GameInstance->bIsGamePaused = false;

    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly());
    UGameplayStatics::SetGamePaused(GetWorld(), false);
	OnGamePausedStateChanged.Broadcast(false);

    UnregisterAndRemoveIMC(UIIMC);
}

bool AC_PlayerController::IsAimingMagicCircle()
{
    return IsValid(MagicCircle);
}

void AC_PlayerController::ShowMagicCircle(UMaterialInterface* DecalMaterial)
{
    if (!IsValid(MagicCircle))
    {
        // 1. Get the current pawn (Ember)
        APawn* ControlledPawn = GetPawn();
        
        // 2. Determine the spawn location (Character's current position)
        // If the pawn is null, we'll default to zero, but we should check it first
        FVector CircleSpawnLocation = ControlledPawn ? ControlledPawn->GetActorLocation() : FVector::ZeroVector;
        FRotator CircleSpawnRotation = FRotator::ZeroRotator;

        // 3. Spawn the Actor at the character's location
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.Instigator = ControlledPawn;

        MagicCircle = GetWorld()->SpawnActor<AC_MagicCircle>(MagicCircleClass, CircleSpawnLocation, CircleSpawnRotation, SpawnParams);
        
        if (DecalMaterial)
            MagicCircle->MagicCircleDecalComponent->SetMaterial(0, DecalMaterial);
    }
}

FVector AC_PlayerController::GetMagicCircleLocation()
{
    return MagicCircle->GetActorLocation();
}

void AC_PlayerController::HideMagicCircle()
{
    if (IsValid(MagicCircle)) 
        MagicCircle->Destroy();
}

void AC_PlayerController::UpdateMagicCircleLocation()
{
    if (!IsValid(MagicCircle)) return;
    
    APawn* P = GetPawn();
    if (!P || !IsValid(MagicCircle)) return;

    // 1. Get Camera Direction (Ignoring Z)
    FVector CamForward = PlayerCameraManager->GetCameraRotation().Vector();
    CamForward.Z = 0;
    CamForward.Normalize();

    // 2. Calculate Position (Player Location + Offset)
    // If Ember is at (0,0,0) and distance is 500, NewLoc should be (500, 0, 0)
    FVector NewLoc = P->GetActorLocation() + (CamForward * CurrentCircleDistance);

    // 3. Force the Location
    MagicCircle->SetActorLocation(NewLoc);
}

void AC_PlayerController::AimMagicCircle(const FInputActionValue& Value)
{
    FVector2D StickInput = Value.Get<FVector2D>();
    
    CurrentCircleDistance = FMath::Clamp(CurrentCircleDistance - (StickInput.Y * MoveSpeed), MinDistance, MaxDistance);
}

void AC_PlayerController::DoOnWaypointActivated(FWaypointDataForMap WaypointInformation)
{
	CachedWaypointInformations.AddUnique(WaypointInformation);
	if (CachedHiddenWaypointInformations.Contains(WaypointInformation))
    {
        CachedHiddenWaypointInformations.Remove(WaypointInformation);
    }
	OnWaypointActivated.Broadcast(WaypointInformation);
}

void AC_PlayerController::DoOnWaypointHidden(FWaypointDataForMap WaypointInformation)
{
	if(CachedWaypointInformations.Contains(WaypointInformation))
    {
        CachedWaypointInformations.Remove(WaypointInformation);
		CachedHiddenWaypointInformations.AddUnique(WaypointInformation);
        OnWaypointHidden.Broadcast(WaypointInformation);
    }
}

void  AC_PlayerController::BroadcastCachedActiveWaypointInformation()
{
    for (const FWaypointDataForMap& WaypointInformation : CachedWaypointInformations)
        OnWaypointActivated.Broadcast(WaypointInformation);
}

void AC_PlayerController::BroadcastCachedHiddenWaypointInformation()
{
    for (const FWaypointDataForMap& WaypointInformation : CachedHiddenWaypointInformations)
        OnWaypointHidden.Broadcast(WaypointInformation);
}

void AC_PlayerController::SwitchCurrentUIOnboardingPhase(EUIOnboardingPhases NewPhase)
{
	CurrentUIOnboardingPhase = NewPhase;
}

void AC_PlayerController::SwitchCurrentPlayerOnboardingPhase(EPlayerOnboardingPhases NewPhase)
{
    CurrentPlayerOnboardingPhase = NewPhase;
}

void AC_PlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
    if (GetASC() == nullptr) return;
    GetASC()->AbilityInputTagPressed(InputTag);
}

void AC_PlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
    if (GetASC() == nullptr) return;
    GetASC()->AbilityInputTagReleased(InputTag);
}

void AC_PlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
    if (GetASC() == nullptr) return;
    GetASC()->AbilityInputTagHeld(InputTag);
}

UASC_AbilitySystemComponent* AC_PlayerController::GetASC()
{
    if (EmberAbilitySystemComponent == nullptr)
        EmberAbilitySystemComponent = Cast<UASC_AbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));

    return EmberAbilitySystemComponent;
}

void AC_PlayerController::CursorTrace()
{
    GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
    if (!CursorHit.bBlockingHit) return;
}

void AC_PlayerController::HandlePauseMenu()
{
	// Prevent opening pause menu if in-game menu is already open
    if (HUD->IsMenuWidgetVisible(EUIWidgetId::InGameMenu)) return;
 
    if (GameInstance->bIsGamePaused)
    {
        ResumeGame();
		HUD->HidePauseMenu();
    }
    else
    {
        PauseGame();
		HUD->ShowPauseMenu(this, GameInstance, GameMode, 1);
    }
}

void AC_PlayerController::HandleInGameMenu()
{
	// Prevent opening in-game menu if pause menu is already open
    if (HUD->IsMenuWidgetVisible(EUIWidgetId::PauseMenu)) return;

    if (GameInstance->bIsGamePaused && HasCompletedEitherUIOnboarding())
    {
        if (CurrentUIOnboardingPhase == EUIOnboardingPhases::CompletedMap) return;
        
        ResumeGame();
		EnableSlateDPadNavigation();
		HUD->HideInGameMenu();
    }

    else if (CurrentUIOnboardingPhase != EUIOnboardingPhases::NotStarted)
    {
        if (HUD->IsMenuWidgetVisible(EUIWidgetId::InGameMenu)) return;

		//TODO: Refactor so that even if we unbind all events attacked to this it destroys after specified preventing this to be called everytime.
		OnInGameMenuOpened.Broadcast();

		PauseGame();
		DisableSlateDPadNavigation();
		HUD->ShowInGameMenu(this, GameInstance, GameMode, nullptr, nullptr, 1);
    }
}

void AC_PlayerController::LoadQuestStateFromGameInstance()
{
    // Find QuestManager component
    QuestManager = FindComponentByClass<UC_QuestManagerComponent>();

    if (QuestManager && GameInstance)
    {
        //Restore quest state from GameInstance if it exists
        QuestManager->LoadQuestState(GameInstance->StoredQuestState);

        if (HUD)
        {
            // This ensures the WidgetController is listening to the current component
            const FWidgetControllerParams WCParams(this, GameInstance, GameMode, nullptr, nullptr);
            HUD->PassQuestWidgetControllerFromIngameToOverlay(WCParams);
        }

        if (UWorld* World = GetWorld())
        {
            if (UC_QuestManagerSubsystem* QuestSubsystem = World->GetSubsystem<UC_QuestManagerSubsystem>())
            {
                QuestSubsystem->RefreshAllObjectives();
            }
        }
    }
}

#pragma region **IMC Tracking**
void AC_PlayerController::EnableCutsceneIMC()
{
    if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();

        //Add only Interaction
        if (CutsceneIMC) Subsystem->AddMappingContext(CutsceneIMC, 0);
    }
}

void AC_PlayerController::DisableCutsceneIMC()
{
    if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->ClearAllMappings();

		// Restore all active contexts
        for (const auto& Context : ActiveContexts)
            Subsystem->AddMappingContext(Context.Key, Context.Value);
    }
}

void AC_PlayerController::RegisterAndAddIMC(UInputMappingContext* NewContext, int32 Priority)
{
    if (!NewContext) return;

    ActiveContexts.Add(NewContext, Priority);
    if (auto* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(NewContext, Priority);
    }
}

void AC_PlayerController::RegisterAndAddIMC(UInputMappingContext* NewContext, UInputMappingContext* OldContext, int32 Priority)
{
    if (OldContext)
        UnregisterAndRemoveIMC(OldContext);

    if (NewContext)
        RegisterAndAddIMC(NewContext, Priority);
}

void AC_PlayerController::UnregisterAndRemoveIMC(UInputMappingContext* ContextToRemove)
{
    if (!ContextToRemove) return;

    // Remove from activeContexts list
    ActiveContexts.Remove(ContextToRemove);

    // remove from the Enhanced Input system
    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (auto* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            Subsystem->RemoveMappingContext(ContextToRemove);
        }
    }
}

void AC_PlayerController::AddMenuIMC()
{
    RegisterAndAddIMC(MenuIMC, 4);
}
#pragma endregion


void AC_PlayerController::DisableSlateDPadNavigation()
{
    if (FSlateApplication::IsInitialized()) 
    {
        TSharedRef<FNavigationConfig> NavConfig = MakeShared<FNavigationConfig>();
        NavConfig->bKeyNavigation = false;
        NavConfig->bAnalogNavigation = false;
        FSlateApplication::Get().SetNavigationConfig(NavConfig);
    }
}

void AC_PlayerController::EnableSlateDPadNavigation()
{
    if (FSlateApplication::IsInitialized()) 
    {
        TSharedRef<FNavigationConfig> NavConfig = MakeShared<FNavigationConfig>();
        NavConfig->bKeyNavigation = true;
        NavConfig->bAnalogNavigation = true;
        FSlateApplication::Get().SetNavigationConfig(NavConfig);
    }
}
 
void AC_PlayerController::EnableDisableInputActionOnPause(UInputAction* InputAction, bool bShouldEnable)
{
    if (!InputAction) return;
    InputAction->bTriggerWhenPaused = bShouldEnable;
}

// Spaghetti code. Can redo this later (probably refactor into a separate music actor component). 
void AC_PlayerController::PlayIdleMusic()
{
    if (!IdleMusic) return;
    if (IdleMusic->IsPlaying()) return;

    IdleMusic->Stop();
    IdleMusic->Play(0.0f);
}

void AC_PlayerController::StopIdleMusic()
{
    if (!IdleMusic) return;
    IdleMusic->FadeOut(MusicFadeOutDuration, 0.0f);
}

void AC_PlayerController::PlayCombatMusic()
{
    if (!CombatMusic) return;
    if (CombatMusic->IsPlaying()) return;

    CombatMusic->Stop();
    CombatMusic->Play(0.0f);
    StopIdleMusic();
}

void AC_PlayerController::StopCombatMusic()
{
    if (!CombatMusic) return;
    CombatMusic->FadeOut(MusicFadeOutDuration, 0.0f);
    PlayIdleMusic();
}