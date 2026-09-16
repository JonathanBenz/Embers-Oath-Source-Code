// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur, Ayushi Parikh

#include "Game/C_GameMode.h"
#include "Components/CapsuleComponent.h"
#include "Game/C_GameInstance.h" 
#include "Game/C_PlayerController.h"
#include "Characters/Player/Ember/C_Ember.h"
#include "Characters/Player/Phoenix/C_Phoenix.h"
#include "UI/C_UIManagerSubsystem.h"
#include "UI/C_InGameWidgetController.h"
#include "Game/C_Structs.h"
#include <Kismet/GameplayStatics.h>
#include "Characters/PS_PlayerState.h"
#include "Sound/SoundClass.h"
#include <GameplayAbilitySystem/Data/DA_AbilityInfo.h>
#include "UI/C_SpellMenuWidgetController.h"
#include "Checkpoint/C_CheckpointSubsystem.h"

void AC_GameMode::BeginPlay()
{
    Super::BeginPlay();

    if (UC_GameInstance* GI = Cast<UC_GameInstance>(GetGameInstance()))
        LoadPlayerGameAbilitiesSavedState(GI->StoredPlayerGameAbilitiesState);
}

AC_GameMode* AC_GameMode::GetCustomGameMode(const UObject* WorldContextObject)
{
	if (WorldContextObject) return Cast<AC_GameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	
	UE_LOG(LogTemp, Error, TEXT("GetCustomGameMode: WorldContextObject is null!"));
	return nullptr;
}

FPlayerGameAbilitiesSavedState AC_GameMode::GetPlayerGameAbilitiesSavedState(bool bIsFromRestart)
{
	// Get the PlayerController and PlayerState to access the current abilities and onboarding phases
    AC_PlayerController* PlayerController = Cast<AC_PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    APS_PlayerState* PlayerState = PlayerController->GetPlayerState<APS_PlayerState>();

    FPlayerGameAbilitiesSavedState SavedState;

    if (!bIsFromRestart)
    {
        if (EmberPawn && PhoenixPawn)
        {
            SavedState.bCanUseLightAttack = EmberPawn->bCanLightAttack;
            SavedState.bCanUseHeavyAttack = EmberPawn->bCanHeavyAttack;
            SavedState.bCanUseDodge = EmberPawn->bCanDodge;
            SavedState.bCanUsePhoenixDodge = PhoenixPawn->bCanDodge;
            SavedState.bCanUseEmberTransform = EmberPawn->bCanTransform;
            SavedState.bCanUsePhoenixTransform = PhoenixPawn->bCanTransform;
			SavedState.bCanUseShoot = PhoenixPawn->bCanLightAttack;
            SavedState.PlayerLevel = EmberPawn->PlayerLevel;
            SavedState.bShownUsingSpells = PlayerController->bShownUsingSpells;

            if (PlayerController && PlayerState)
            {
                SavedState.CurrentUIOnboardingPhase = PlayerController->CurrentUIOnboardingPhase;
                SavedState.CurrentOnboardingPhase = PlayerController->CurrentPlayerOnboardingPhase;
                SavedState.OathPoints = PlayerState->GetSpellPoints();

                if (UASC_AbilitySystemComponent* ASC = Cast<UASC_AbilitySystemComponent>(PlayerState->GetAbilitySystemComponent()))
                {
                    FScopedAbilityListLock Lock(*ASC);
                    for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
                    {
                        FGameplayTag StatusTag = ASC->GetStatusFromSpec(Spec);
                        FGameplayTag AbilityTag = ASC->GetAbilityFromSpec(Spec);

                        // Skip abilities the player hasn't spent points on yet
                        bool bIsLocked = StatusTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Locked")));
                        bool bIsEligible = StatusTag.MatchesTagExact(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Eligible")));
                        if (bIsLocked || bIsEligible || !AbilityTag.IsValid()) continue;

                        FEquippedAbilitySlot Slot;
                        Slot.AbilityTag = AbilityTag;
                        Slot.StatusTag = StatusTag;
                        Slot.AbilityLevel = Spec.Level;
                        Slot.SlotInputTag = ASC->GetInputTagFromSpec(Spec); // Empty if just Unlocked

                        SavedState.EquippedAbilitySlots.Add(Slot);
                    }
                }
            }
        }
    }

    for (const FEquippedAbilitySlot& Slot : SavedState.EquippedAbilitySlots)
    {
        UE_LOG(LogTemp, Warning, TEXT("SAVING slot: Ability=%s Status=%s Input=%s Level=%d"),
            *Slot.AbilityTag.ToString(),
            *Slot.StatusTag.ToString(),
            *Slot.SlotInputTag.ToString(),
            Slot.AbilityLevel);
    }

    return SavedState;
}

void AC_GameMode::LoadPlayerGameAbilitiesSavedState(const FPlayerGameAbilitiesSavedState& SavedState)
{
	// Get the PlayerController and PlayerState to apply the saved abilities and onboarding phases
    AC_PlayerController* PlayerController = Cast<AC_PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    APS_PlayerState* PlayerState = PlayerController->GetPlayerState<APS_PlayerState>();

    if (EmberPawn && PhoenixPawn)
    {
        EmberPawn->bCanLightAttack = SavedState.bCanUseLightAttack;
        EmberPawn->bCanHeavyAttack = SavedState.bCanUseHeavyAttack;
        EmberPawn->bCanDodge = SavedState.bCanUseDodge;
        EmberPawn->bCanTransform = SavedState.bCanUseEmberTransform;
        PhoenixPawn->bCanTransform = SavedState.bCanUsePhoenixTransform;
        PhoenixPawn->bCanLightAttack = SavedState.bCanUseShoot;
        PhoenixPawn->bCanDodge = SavedState.bCanUsePhoenixDodge;
		EmberPawn->PlayerLevel = SavedState.PlayerLevel;
        PlayerController->bShownUsingSpells = SavedState.bShownUsingSpells;

        if (PlayerController && PlayerState)
        {
            PlayerController->CurrentUIOnboardingPhase = SavedState.CurrentUIOnboardingPhase;
            PlayerController->CurrentPlayerOnboardingPhase = SavedState.CurrentOnboardingPhase;
			PlayerState->SetSpellPoints(SavedState.OathPoints);

            UASC_AbilitySystemComponent* ASC = Cast<UASC_AbilitySystemComponent>(PlayerState->GetAbilitySystemComponent());
		}
	}
}

void AC_GameMode::SpawnEmberOnGround()
{
	UWorld* World = GetWorld();
	if (!World) return;

	FVector StartLocation = PhoenixPawn->GetActorLocation();
	FVector TraceEnd = StartLocation - FVector(0, 0, 10000.0f); // Trace downwards

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Ignore the spawner itself
	QueryParams.bTraceComplex = false;

	bool bHit = World->LineTraceSingleByChannel(HitResult, StartLocation, TraceEnd, ECC_Visibility, QueryParams);

	if (bHit)
	{
		// Set the Z to the exact point the trace hit the ground
		FVector SpawnLocation = HitResult.ImpactPoint;

		// Adjust for the capsule half-height to ensure Ember is standing on the ground
		float CapsuleHalfHeight = EmberPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		SpawnLocation.Z += CapsuleHalfHeight;
		SpawnLocation.Z += 2.0f; // Small offset to prevent clipping into the ground

		EmberPawn->SetActorLocation(SpawnLocation);
	}
}

void AC_GameMode::RestorePlayerSpellAbilities()
{
    if (UC_GameInstance* GI = Cast<UC_GameInstance>(GetGameInstance()))
    {
        const FPlayerGameAbilitiesSavedState& SavedState = GI->StoredPlayerGameAbilitiesState;
        if (SavedState.EquippedAbilitySlots.Num() > 0)
        {
            APS_PlayerState* PS = EmberPawn->GetPlayerState<APS_PlayerState>();
            if (PS)
            {
                UASC_AbilitySystemComponent* ASC = Cast<UASC_AbilitySystemComponent>(
                    PS->GetAbilitySystemComponent());

                if (ASC)
                {
                    FTimerHandle RestoreHandle;
                    TArray<FEquippedAbilitySlot> SlotsCopy = SavedState.EquippedAbilitySlots;

                    GetWorldTimerManager().SetTimerForNextTick([this, ASC, SlotsCopy]() // Does this need to be a lambda instead of its own function? 
                    {
                        if (!IsValid(this) || !IsValid(ASC)) return; //Safety Check

                        UE_LOG(LogTemp, Warning, TEXT("Restoring %d ability slots"), SlotsCopy.Num());

                        for (const FEquippedAbilitySlot& Slot : SlotsCopy)
                        {
                            if (!Slot.AbilityTag.IsValid()) continue;

                            FGameplayAbilitySpec* Spec = ASC->GetSpecFromAbilityTag(Slot.AbilityTag);
                            if (!Spec)
                            {
                                UE_LOG(LogTemp, Warning, TEXT("Spec not found for %s"), *Slot.AbilityTag.ToString());
                                continue;
                            }

                            Spec->Level = Slot.AbilityLevel;
                            Spec->DynamicAbilityTags.RemoveTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Eligible")));
                            Spec->DynamicAbilityTags.RemoveTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status.Locked")));

                            Spec->DynamicAbilityTags.AddTag(Slot.StatusTag);
                            ASC->MarkAbilitySpecDirty(*Spec);

                            ASC->ClientUpdateAbilityStatus(Slot.AbilityTag, Slot.StatusTag, Slot.AbilityLevel);

                            if (Slot.SlotInputTag.IsValid())
                            {
                                UE_LOG(LogTemp, Warning, TEXT("Restoring %s -> %s"),
                                    *Slot.AbilityTag.ToString(), *Slot.SlotInputTag.ToString());
                                ASC->ServerEquipAbility(Slot.AbilityTag, Slot.SlotInputTag);
                            }
                        }
                    }); // End of Lambda

                }
            }
        }
    }
}

APawn* AC_GameMode::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	APawn* SpawnedPawn = Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (CurrentPlayerForm == EPlayerForms::Ember)
	{
		EmberPawn = Cast<AC_Ember>(SpawnedPawn);
		PhoenixPawn = GetWorld()->SpawnActor<AC_Phoenix>(PhoenixBP, SpawnTransform, SpawnParams);

		if(EmberPawn && PhoenixPawn)
			SwitchForm(EPlayerForms::Ember);
	}

	if (CurrentPlayerForm == EPlayerForms::Phoenix)
	{
		PhoenixPawn = Cast<AC_Phoenix>(SpawnedPawn);
		EmberPawn = GetWorld()->SpawnActor<AC_Ember>(EmberBP, SpawnTransform, SpawnParams);

		if (EmberPawn && PhoenixPawn)
			SwitchForm(EPlayerForms::Phoenix);
	}

	if (EmberPawn && PhoenixPawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spawned Pawns: Ember - %s, Phoenix - %s"), *EmberPawn->GetName(), *PhoenixPawn->GetName());
		RestorePlayerSpellAbilities();
	}
	else
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn one or more pawns!"));

	return SpawnedPawn;
}

void AC_GameMode::SwitchForm(EPlayerForms NewForm)
{
    if (EmberPawn == nullptr || PhoenixPawn == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("SwitchForm: One or both player forms are null!"));
        return;
    }

    if (AC_PlayerController* PlayerController = Cast<AC_PlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0)))
    {
        // Determine which pawns are the 'Old' and 'New' for this transition
        AC_BasePlayer* OldPawn = (NewForm == EPlayerForms::Ember) ? (AC_BasePlayer*)PhoenixPawn : (AC_BasePlayer*)EmberPawn;
        AC_BasePlayer* NewPawn = (NewForm == EPlayerForms::Ember) ? (AC_BasePlayer*)EmberPawn : (AC_BasePlayer*)PhoenixPawn;

        //Broadcast form-specific UI or Event changes
        OnIsPhoenixCharActive.Broadcast(NewForm == EPlayerForms::Phoenix);

        //Position the new form based on the old form's location/rotation
        if (NewForm == EPlayerForms::Ember)
        {
            SpawnEmberOnGround();
            FRotator CurrentRotation = PhoenixPawn->GetActorRotation();
            CurrentRotation.Pitch = 0.0f;
            CurrentRotation.Roll = 0.0f;
            EmberPawn->SetActorRotation(CurrentRotation);

        	if (EmberPawn->OnTransformedEffectToBeApplied)
        	{
        		FGameplayEffectContextHandle Context = EmberPawn->GetAbilitySystemComponent()->MakeEffectContext();
        		Context.AddSourceObject(this);

        		FGameplayEffectSpecHandle SpecHandle = EmberPawn->GetAbilitySystemComponent()->MakeOutgoingSpec(EmberPawn->OnTransformedEffectToBeApplied, 1.0f, Context);
        		if (SpecHandle.IsValid())
        		{
        			EmberPawn->GetAbilitySystemComponent()->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
        			UE_LOG(LogTemp, Display, TEXT("Successfully applied Ember Transformation Effect!"));
        		}
        	}
        }

        else PhoenixPawn->SetActorRotation(EmberPawn->GetActorRotation());

        // Update Pawn Visibility and Physics States
        OldPawn->SetActorHiddenInGame(true);
        OldPawn->SetActorEnableCollision(false);
        OldPawn->SetActorTickEnabled(false);
        
        NewPawn->SetActorHiddenInGame(false);
        NewPawn->SetActorEnableCollision(true);
        NewPawn->SetActorTickEnabled(true);

        // Form-Specific Logic (Sound/Movement)
        if (NewForm == EPlayerForms::Phoenix)
        {
            if (PhoenixSoundClass) PhoenixSoundClass->Properties.Volume = 1.0f;
            PhoenixPawn->GetMesh()->bPauseAnims = false;
            PhoenixPawn->bCanMovePerpetual = true;
            EmberPawn->HideAllAttachedItems();
        }

        else // else Ember
        {
            if (PhoenixSoundClass) PhoenixSoundClass->Properties.Volume = 0.0f;
            PhoenixPawn->GetMesh()->bPauseAnims = true;
            PhoenixPawn->bCanMovePerpetual = false;

            if (EmberPawn->GetCharacterState() == ECharacterState::ECS_EquippedOneHandedWeapon)
                EmberPawn->ShowAllAttachedItems();
        }

        PlayerController->Possess(NewPawn);

        // Update Input Mapping Context for the new form
        UInputMappingContext* OldContext = (NewForm == EPlayerForms::Ember) ? PhoenixPawn->PhoenixInputMappingContext : EmberPawn->EmberInputMappingContext;
        UInputMappingContext* NewContext = (NewForm == EPlayerForms::Ember) ? EmberPawn->EmberInputMappingContext : PhoenixPawn->PhoenixInputMappingContext;
		PlayerController->RegisterAndAddIMC(NewContext, OldContext, 0); // Assuming priority 0 for new context

        // Re-Initialize GAS on the New Avatar
        if (APS_PlayerState* PS = NewPawn->GetPlayerState<APS_PlayerState>())
        {
            if (UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent())
            {
                // Link the PlayerState (Owner) to the New Pawn (Avatar)
                ASC->InitAbilityActorInfo(PS, NewPawn);

                // Sync the Pawn's local pointers to the PlayerState's source of truth
                NewPawn->PlayerAbilitySystemComponent = ASC;
                NewPawn->PlayerAttributeSet = PS->GetAttributeSet();
            }
        }

        CurrentPlayerForm = NewForm;
    }

    else UE_LOG(LogTemp, Error, TEXT("SwitchForm: PlayerController is null!"));
}