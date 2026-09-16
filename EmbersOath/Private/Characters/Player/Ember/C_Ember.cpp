// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Hritish Duvvur, Ayushi Parikh

#include "Characters/Player/Ember/C_Ember.h"
#include "Characters/Player/Phoenix/C_Phoenix.h"
#include "Characters/PS_PlayerState.h"
#include "Game/C_GameMode.h"
#include "Game/C_Enums.h"
#include "Game/C_HelperFunctionLibrary.h"	
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "InputAction.h"
#include "Items/Weapons/C_Weapon.h"
#include "Items/C_Item.h"
#include "Game/C_PlayerController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Game/C_GameInstance.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "Checkpoint/C_CheckpointSubsystem.h"

AC_Ember::AC_Ember()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->TargetArmLength = DefaultArmLength;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

void AC_Ember::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSpentAsEmber += DeltaTime;
	PlayerLocation = GetActorLocation();
	PlayerCameraYaw = Camera->GetComponentRotation().Yaw;
	
	//This logic is for smoothly transitioning the camera when casting Fire Rain. We interpolate both the arm length and the rotation of the spring arm to create a smooth zoom out and angle change.
	float TargetLength = bIsCastingFireRain ? FireRainArmLength : DefaultArmLength;
	FRotator TargetRot = bIsCastingFireRain ? FireRainRotation : FRotator::ZeroRotator;

	// Interpolate Length
	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, TargetLength, DeltaTime, SpellCamInterpSpeed);

	// Interpolate Rotation
	FRotator CurrentRot = SpringArm->GetRelativeRotation();
	FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, SpellCamInterpSpeed);
	SpringArm->SetRelativeRotation(NewRot);

	CheckIfShouldInterpToCombatFOV(DeltaTime);
}

void AC_Ember::BeginPlay()
{
	Super::BeginPlay();
	EnableEnhancedInputMapping(EmberInputMappingContext);
	SpawnDefaultWeapon();
	SheatheWeapon();
	check(DodgeMontage);

	CachedPlayerState = Cast<APS_PlayerState>(UGameplayStatics::GetPlayerState(this, 0));
}

void AC_Ember::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	if (EndPlayReason == EEndPlayReason::Type::EndPlayInEditor || 
		EndPlayReason == EEndPlayReason::Type::Quit)
	{
		if (UC_GameInstance* MyGI = Cast<UC_GameInstance>(UGameplayStatics::GetGameInstance(this)))
			MyGI->SetEmberCombatStats(LightAttackCount, HeavyAttackCount, TimeSpentAsEmber, TransformToPhoenixCount); // Attack Counts of Ember
	}
}

void AC_Ember::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AC_Ember::OnMoveEnded);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Canceled, this, &AC_Ember::OnMoveEnded);
	}
}

void AC_Ember::Move(const FInputActionValue& Value)
{
	if (!Controller) return;
	
	const FVector2D CurrentValue = Value.Get<FVector2D>();
	if (CurrentValue.GetSafeNormal().Size() < 0.2f) return; // If there is small controller drift, don't move. 

	const FRotator ControllerRotation = Controller->GetControlRotation();
	const FRotator YRotation(0.0f, ControllerRotation.Yaw, 0.0f);

	const FVector ForwardDirection = UKismetMathLibrary::GetForwardVector(YRotation);
	AddMovementInput(ForwardDirection, CurrentValue.Y);

	const FRotator XRotation(0.0f, ControllerRotation.Yaw, ControllerRotation.Roll);
	const FVector RightDirection = UKismetMathLibrary::GetRightVector(XRotation);
	AddMovementInput(RightDirection, CurrentValue.X);

	MoveInput.X = CurrentValue.X;
	MoveInput.Y = CurrentValue.Y;
	bIsMoving = true;
}

/// This Look is overriding the BasePlayer method because there is Sprint functionality embedded here.
void AC_Ember::Look(const FInputActionValue& Value)
{
	AC_PlayerController* PC = Cast<AC_PlayerController>(GetController());
	if (!PC) return;

	const FVector2D InputAxisValue = Value.Get<FVector2D>();
	float CurrentYawSensitivityValue = PC->GetCameraYawSensitivity();
	float CurrentPitchSensitivityValue = PC->GetCameraPitchSensitivity();

	if (PC->IsAimingMagicCircle())
	{
		// 1. Let the Camera Pivot (Yaw)
		// This rotates the Controller, which rotates the SpringArm, causing the circle to orbit relative to the camera.
		AddControllerYawInput(InputAxisValue.X * CurrentYawSensitivityValue * AttackInputAxisModifyValue);

		// 2. Redirect Vertical Input (Pitch) to move the circle Closer/Farther. Only the Y value to handle distance.
		PC->AimMagicCircle(Value); 
		return; 
	}

	if (bIsCurrentlyTargetLocking) return; // Don't want to add any controller rotation input if using target locking. 

	if (CurrentGate == ECurrentGate::ECG_Sprint)
	{
		FVector2D SprintInputAxisModifier(InputAxisValue * 0.2f);
		AddControllerYawInput(SprintInputAxisModifier.X * CurrentYawSensitivityValue);
		AddControllerPitchInput(SprintInputAxisModifier.Y * CurrentPitchSensitivityValue);
	}
	else if (bIsAttacking)
	{
		FVector2D AttackInputAxisModifier(InputAxisValue * AttackInputAxisModifyValue);
		AddControllerYawInput(AttackInputAxisModifier.X * CurrentYawSensitivityValue);
		AddControllerPitchInput(AttackInputAxisModifier.Y * CurrentPitchSensitivityValue);
	}
	else
	{
		AddControllerYawInput(InputAxisValue.X * CurrentYawSensitivityValue);
		AddControllerPitchInput(InputAxisValue.Y * CurrentPitchSensitivityValue);
	}
}

void AC_Ember::Die_Implementation()
{
	Super::Die_Implementation();
	HideAllAttachedItems();
}

void AC_Ember::TurnAroundAtWorldBounds(const FVector& TurnDir)
{
	if (AC_PlayerController* PC = Cast<AC_PlayerController>(GetController())) 
	{ 
		DisableInput(PC); 
		PC->SetIgnoreLookInput(true);
	}

	TurnAroundVector = TurnDir;
	TurnAroundRotation = TurnDir.Rotation();
	GetWorldTimerManager().SetTimer(TurnAroundTimerHandle, this, &AC_Ember::UpdateTurnAround, 0.01f, true);
}

/// <summary>
/// Helper function for TurnAroundAtWorldBounds() func.
/// </summary>
void AC_Ember::UpdateTurnAround()
{
	if (!bIsMovingForward) // If not moving forward, e.g., if turning.
	{
		float Dot = FVector::DotProduct(GetActorForwardVector(), TurnAroundVector);
		
		// Base Case: Both vectors are pointed in the same direction. 
		if (Dot >= 0.9f)
		{
			bIsMovingForward = true;
			GetWorldTimerManager().SetTimer(FinishTurnAroundTimerHandle, this, &AC_Ember::FinishTurnAround, 2.5f, false);
			return;
		}

		FRotator NewRot = FMath::RInterpTo(GetActorRotation(), TurnAroundRotation, GetWorld()->GetDeltaSeconds(), 2.0f);
		GetController()->SetControlRotation(NewRot);
	}

	else if (bIsMovingForward)
		AddMovementInput(GetActorForwardVector(), 1.0f);
}

/// <summary>
/// Helper function for UpdateTurnAround() func. 
/// </summary>
void AC_Ember::FinishTurnAround()
{
	bIsMovingForward = false;
	if (AC_PlayerController* PC = Cast<AC_PlayerController>(GetController()))
	{
		EnableInput(PC);
		PC->ResetIgnoreLookInput();
	}
	GetWorldTimerManager().ClearTimer(TurnAroundTimerHandle);
	GetWorldTimerManager().ClearTimer(FinishTurnAroundTimerHandle);
}

void AC_Ember::StartFireRainCamera_Implementation()
{
	// Save current length so we know exactly where to return
	// DefaultArmLength = SpringArm->TargetArmLength;
	bIsCastingFireRain = true;
    
	// Disable Control Rotation so the player can't "fight" the top-down view
	SpringArm->bUsePawnControlRotation = false;
	SpringArm->bInheritPitch = false; 
}

void AC_Ember::EndFireRainCamera_Implementation()
{
	bIsCastingFireRain = false;
    
	// Restore control once the camera is back to normal
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bInheritPitch = true;
}

FName AC_Ember::GetDodgeDirectionSectionName()
{
	if (!bIsMoving) return FName("Backstep"); // If no movement, player does a backstep instead of rolling. 

	const float DodgeAngle = CalculateDodgeAngle();
	if (-22.5f <= DodgeAngle && DodgeAngle <= 22.5f)
	{
		if (FMath::RandBool())       // Will randomly choose which variant of the Forward Roll to play. 
			return FName("Fwd");
		else
			return FName("Fwd_Alt");
	}

	else if (22.5f < DodgeAngle && DodgeAngle < 67.5f)
		return FName("Fwd_Right");

	else if (67.5f <= DodgeAngle && DodgeAngle <= 112.5f)
		return FName("Right");

	else if (112.5f < DodgeAngle && DodgeAngle < 157.5f)
		return FName("Bwd_Right");

	else if (DodgeAngle >= 157.5f || DodgeAngle <= -157.5f)
		return FName("Bwd");
	else if (-67.5f < DodgeAngle && DodgeAngle < -22.5f)
		return FName("Fwd_Left");

	else if (-112.5f <= DodgeAngle && DodgeAngle <= -67.5f)
		return FName("Left");

	else if (-157.5f < DodgeAngle && DodgeAngle < -112.5f)
		return FName("Bwd_Left");

	return FName("Fwd"); // Fallback Value if everything else fails. If this is returning, then something wrong is happening. 
}

float AC_Ember::CalculateDodgeAngle()
{
	const FVector2D Dir = MoveInput.GetSafeNormal();
	const float AngleRadians = FMath::Atan2(Dir.X, Dir.Y);
	return FMath::RadiansToDegrees(AngleRadians); // In range of -180 to +180 degrees
}

void AC_Ember::Transform()
{
	TransformToPhoenixCount++; //Count the number of times the player has transformed for to Phoenix
	AC_GameMode::GetCustomGameMode(this)->SwitchForm(EPlayerForms::Phoenix);	
}

void AC_Ember::PlayEquipMontage(const FName& SectionName)
{
	if (TObjectPtr<UAnimInstance> AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (EquipMontage)
		{
			AnimInstance->Montage_Play(EquipMontage);
			AnimInstance->Montage_JumpToSection(SectionName);
		}
	}
}

void AC_Ember::CheckIfShouldInterpToCombatFOV(float DeltaTime)
{
	if (!CachedPlayerState) return;
	if (!CachedPlayerState->IsInCombat()) return;

	float NewFOV = FMath::FInterpTo(Camera->FieldOfView, CombatFOV, DeltaTime, CombatFOVInterpSpeed);
	if (FMath::IsNearlyEqual(NewFOV, CombatFOV, 0.1f)) NewFOV = CombatFOV;

	Camera->SetFieldOfView(NewFOV);
}

void AC_Ember::Equip()
{
	if (CanSheatheWeapon()) SheatheWeapon(); // If not in the middle of an attack, and has some type of weapon equipped, sheathe it
	else if (CanDrawWeapon()) DrawWeapon();  // Else, draw out the sheathed weapon
}

void AC_Ember::DrawWeapon()
{
	ShowWeapon();
	AttachWeaponToHand();
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
}

void AC_Ember::SheatheWeapon()
{
	CharacterState = ECharacterState::ECS_NoWeaponEquipped;
	HideWeapon();
}

void AC_Ember::SetOverlappingItem(TObjectPtr<AC_Item> Item) { OverlappingItem = Item; }
void AC_Ember::AttachWeaponToHand() { EquippedWeapon->EquipToPlayer(GetMesh(), FName("WeaponSocket"), this, this); }
void AC_Ember::ShowWeapon() { EquippedWeapon->SetActorHiddenInGame(false); }
void AC_Ember::HideWeapon() { EquippedWeapon->SetActorHiddenInGame(true); }
bool AC_Ember::CanSheatheWeapon() { return CharacterState != ECharacterState::ECS_NoWeaponEquipped && EquippedWeapon; }
bool AC_Ember::CanDrawWeapon() { return CharacterState == ECharacterState::ECS_NoWeaponEquipped && EquippedWeapon; }

void AC_Ember::HideAllAttachedItems()
{
	if (!EquippedWeapon) return;
	EquippedWeapon->SetActorHiddenInGame(true);
	EquippedWeapon->SetActorEnableCollision(false);
	EquippedWeapon->SetActorTickEnabled(false);
}

void AC_Ember::ShowAllAttachedItems()
{
	if (!EquippedWeapon) return;
	EquippedWeapon->SetActorHiddenInGame(false);
	EquippedWeapon->SetActorEnableCollision(true);
	EquippedWeapon->SetActorTickEnabled(true);
}

float AC_Ember::CalculateDeltaYawFromCamera()
{
	PreviousCamYaw = CurrentCamYaw;
	CurrentCamYaw = PlayerCameraYaw;
	return CurrentCamYaw - PreviousCamYaw;
}