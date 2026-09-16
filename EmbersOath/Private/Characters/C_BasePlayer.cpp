// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Hritish Duvvur, Ayushi Parikh

#include "Characters/C_BasePlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Game/C_PlayerController.h"
#include "Game/C_GameMode.h"
#include "Game/C_GameInstance.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Characters/PS_PlayerState.h"
#include "UI/C_HUD.h"
#include "Game/C_HelperFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

AC_BasePlayer::AC_BasePlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AC_BasePlayer::BeginPlay()
{
	Super::BeginPlay();
	InitializeHUDOverlay();
	if (GetWorld()) GetWorldTimerManager().SetTimer(PhysicalMaterialGroundTraceTimer, this, &AC_BasePlayer::TrackGroundPhysicalMaterial, 0.25f, true);
	Tags.Add(FName("EngageableTarget"));
	
	if (UASC_AbilitySystemComponent* ASC = Cast<UASC_AbilitySystemComponent>(GetAbilitySystemComponent()))
		ASC->UpdateAbilityStatus(PlayerLevel);
}

void AC_BasePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AC_BasePlayer::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (bCharacterIsInitialized) return; // Do not re-initialize anything GAS related upon re-possesion! 

	InitAbilityActorInfo();
	GiveDefaultAbilities();
}

void AC_BasePlayer::InitAbilityActorInfo()
{
	APS_PlayerState* EmbersOathPlayerState = GetPlayerState<APS_PlayerState>();
	if (EmbersOathPlayerState)
		EmbersOathPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(EmbersOathPlayerState, this); // Initialize the ASC

	Cast<UASC_AbilitySystemComponent>(EmbersOathPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	PlayerAbilitySystemComponent = EmbersOathPlayerState->GetAbilitySystemComponent();
	PlayerAttributeSet = EmbersOathPlayerState->GetAttributeSet();
	
	InitializeAttributes();
}

void AC_BasePlayer::InitializeAttributes()
{
	if (PlayerAbilitySystemComponent && DefaultAttributeEffect)
	{
		FGameplayEffectContextHandle EffectContext = PlayerAbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		FGameplayEffectSpecHandle SpecHandle = PlayerAbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeEffect, 1, EffectContext);

		if (SpecHandle.IsValid())
			FActiveGameplayEffectHandle GEHandle = PlayerAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		bCharacterIsInitialized = true;
	}
}

void AC_BasePlayer::PlayerLevelUp(bool bShouldAddSpellPoints)
{
	if (UASC_AbilitySystemComponent* ASC = Cast<UASC_AbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		PlayerLevel += 1;
		ASC->UpdateAbilityStatus(PlayerLevel);

		if (AC_PlayerController* EmberPlayerController = Cast<AC_PlayerController>(GetController()))
			EmberPlayerController->OnPlayerLevelUp.Broadcast(0, bShouldAddSpellPoints);
	}
	
	if (APS_PlayerState* PS = Cast<APS_PlayerState>(GetPlayerState()))
	{
		if (bShouldAddSpellPoints) PS->AddSpellPoints(1);
	}
}

void AC_BasePlayer::ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial)
{
	if (AC_PlayerController* EmberPlayerController = Cast<AC_PlayerController>(GetController()))
		EmberPlayerController->ShowMagicCircle(DecalMaterial);
}

FVector AC_BasePlayer::GetMagicCircleLocation_Implementation() { return MagicCircleLocation; }

void AC_BasePlayer::HideMagicCircle_Implementation()
{
	if (AC_PlayerController* EmberPlayerController = Cast<AC_PlayerController>(GetController()))
	{
		MagicCircleLocation = EmberPlayerController->GetMagicCircleLocation();
		EmberPlayerController->HideMagicCircle();
	}
}

void AC_BasePlayer::InitializeHUDOverlay()
{
	AC_PlayerController* PlayerController = Cast<AC_PlayerController>(GetController());
	AC_GameMode* GameMode = Cast<AC_GameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	UC_GameInstance* GameInstance = Cast<UC_GameInstance>(GetWorld()->GetGameInstance());
	
	if (PlayerController && GameInstance && GameMode)
	{
		if (AC_HUD* HUD = Cast<AC_HUD>(PlayerController->GetHUD()))
		{
			//Check if level loaded is not MainMenu level to avoid initializing HUD overlay twice in editor when PIE starts from MainMenu level
			if (GetWorld()->GetName() != UC_HelperFunctionLibrary::GetLevelNameFromEnum(ELevelNames::MainMenu).ToString())
				HUD->ShowOverlay(PlayerController, GameInstance, GameMode, PlayerAbilitySystemComponent, PlayerAttributeSet);
		}
	}
}

void AC_BasePlayer::EnableEnhancedInputMapping(TObjectPtr<UInputMappingContext> InputMappingContext)
{
	if (TObjectPtr<AC_PlayerController> PlayerController = Cast<AC_PlayerController>(GetController()))
		PlayerController->RegisterAndAddIMC(InputMappingContext, 0);
}

void AC_BasePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AC_BasePlayer::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AC_BasePlayer::Look);
	}
}

void AC_BasePlayer::Die_Implementation()
{
	Super::Die_Implementation();
	if (DeathSound) UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation());
	GetWorldTimerManager().SetTimer(EmberDeathTimerHandle, this, &AC_BasePlayer::RespawnAfterDeath, TimeUntilMenuLoad);
}

void AC_BasePlayer::RespawnAfterDeath()
{
	if (AC_PlayerController* PlayerController = Cast<AC_PlayerController>(GetController()))
	{
		PlayerController->ResumeGame();
		PlayerController->OnRespawnAfterDeath();
	}
}

/// <summary>
/// Trace downward and see if the player is standing on a physical material (e.g., grass, dirt, mud, stone, etc.). 
/// Cache that value in the var GroundPhysicalMaterial. 
/// </summary>
void AC_BasePlayer::TrackGroundPhysicalMaterial()
{
	if (GetWorld())
	{
		FHitResult OutHit;
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(this);
		const float PlayerHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const FVector Start = GetActorLocation() - FVector(0.0f, 0.0f, PlayerHalfHeight); // Start at bottom of player capsule
		const FVector End = FVector(Start.X, Start.Y, Start.Z - 100.0f); // Trace 1m downward

		bool bHitResult = UKismetSystemLibrary::SphereTraceSingle(
			GetWorld(), Start, End, 1.0f, UEngineTypes::ConvertToTraceType(ECC_Visibility),
			false, ActorsToIgnore, EDrawDebugTrace::None, OutHit, true);

		GroundPhysMaterial = bHitResult ? OutHit.PhysMaterial : nullptr;
	}
}

// Camera shake logic
UCameraShakeBase* AC_BasePlayer::PlayCameraShake(TSubclassOf<UCameraShakeBase> CameraShakeBlueprint, UCameraShakeBase* CameraShakeInstance)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (!CameraShakeInstance)
			return PC->PlayerCameraManager->StartCameraShake(CameraShakeBlueprint);
	}

	return nullptr;
}

void AC_BasePlayer::StopCameraShake(UCameraShakeBase* CameraShakeInstance)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
		PC->PlayerCameraManager->StopCameraShake(CameraShakeInstance, false);
}