// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Ayushi Parikh

#include "Characters/Player/Phoenix/C_Phoenix.h"
#include "Characters/Player/Phoenix/C_PhoenixCameraComponent.h"
#include "Game/C_GameMode.h"
#include "Game/C_GameInstance.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UI/C_UIManagerSubsystem.h"
#include "UI/C_OptionsWidgetController.h"
#include <Engine/Engine.h>
#include <Engine/World.h>
#include <GameFramework/Character.h>
#include <Kismet/GameplayStatics.h>
#include "EnhancedInputSubsystems.h"
#include "Characters/PS_PlayerState.h"

AC_Phoenix::AC_Phoenix()
{
	PrimaryActorTick.bCanEverTick = true;

	// Camera setup
	CameraPivot = CreateDefaultSubobject<USceneComponent>(TEXT("Camera Pivot"));
	CameraPivot->SetupAttachment(GetRootComponent());

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(CameraPivot);
	SpringArm->bUsePawnControlRotation = false;

	CameraComponent = CreateDefaultSubobject<UC_PhoenixCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArm);

	// Visual setup
	VisualPivot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualPivot"));
	VisualPivot->SetupAttachment(GetRootComponent());
	GetMesh()->SetupAttachment(VisualPivot);

	bUseControllerRotationPitch = true;
	bUseControllerRotationRoll = true;
	bUseControllerRotationYaw = true;
	GetCharacterMovement()->bOrientRotationToMovement = false;

	//Set default values
	SpringArm->TargetArmLength = DefaultSpringArmLength;
	GetCharacterMovement()->MaxFlySpeed = MaxFlySpeed;
	GetCharacterMovement()->MaxAcceleration = DefaultAccelerationSpeed;
	GetCharacterMovement()->BrakingDecelerationFlying = DefaultDecelerationSpeed;

	bCanMovePerpetual = true;
}

void AC_Phoenix::BeginPlay()
{
	Super::BeginPlay();
	EnableEnhancedInputMapping(PhoenixInputMappingContext);

	// Initial sync + bind to Options Controller invert-controls changes
	if (UC_UIManagerSubsystem* UIManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_UIManagerSubsystem>())
	{
		if (UC_OptionsWidgetController* OptionsCtrl = UIManager->GetRegisteredControllerTyped<UC_OptionsWidgetController>(EUIWidgetId::Options))
		{
			bInvertPitchControl = OptionsCtrl->GetInvertUpDownControls();
			OptionsCtrl->OnInvertControlsChanged.AddDynamic(this, &AC_Phoenix::HandleInvertControlsChanged);

			CameraComponent->SetInvertCameraControls(OptionsCtrl->GetInvertCameraControls());
			OptionsCtrl->OnInvertCameraControlsChanged.AddDynamic(CameraComponent, &UC_PhoenixCameraComponent::SetInvertCameraControls);
		}
	}
	GetCharacterMovement()->SetMovementMode(MOVE_Flying);
}

void AC_Phoenix::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSpentAsPhoenix += DeltaTime;
	PlayerLocation = GetActorLocation();
	PlayerCameraYaw = CameraComponent->GetComponentRotation().Yaw;

	if (bCanMovePerpetual)
	{
		GroundSphereTrace();
		PerpetualMotion(DeltaTime);
		HandleYawAndRoll(DeltaTime);
		ResetMeshRotation(DeltaTime);
	}
}

void AC_Phoenix::TurnAround(const FVector& TurnDir)
{
	if (AC_PlayerController* PC = Cast<AC_PlayerController>(GetController())) DisableInput(PC);

	TurnAroundVector = TurnDir;
	if (!bIsBoosting) BoostStarted();

	// Should the Phoenix steer to the left or the right when turning around?
	TurnSteeringDir = FVector::CrossProduct(GetActorForwardVector(), TurnAroundVector).Z;
	if (TurnSteeringDir < 0.0f) TurnSteeringDir = -1.0f;
	else TurnSteeringDir = 1.0f;

	GetWorldTimerManager().SetTimer(TurnAroundTimerHandle, this, &AC_Phoenix::UpdateTurnAround, 0.01f, true);
}

void AC_Phoenix::UpdateTurnAround()
{
	float Dot = FVector::DotProduct(GetActorForwardVector(), TurnAroundVector);
	
	// Base Case: Both vectors are pointed in the same direction. 
	if (Dot >= 0.95f)
	{
		BoostCompleted();
		RollMovementInput = 0.0f;
		GetWorldTimerManager().ClearTimer(TurnAroundTimerHandle);
		GetWorldTimerManager().SetTimer(FinishTurnAroundTimerHandle, this, &AC_Phoenix::FinishTurnAround, 2.5f, false);
		return;
	}

	RollMovementInput = TurnSteeringDir;
}

/// <summary>
/// This function exists because we want some additional delay before giving control back to the player. 
/// </summary>
void AC_Phoenix::FinishTurnAround()
{
	if (AC_PlayerController* PC = Cast<AC_PlayerController>(GetController())) EnableInput(PC);
	GetWorldTimerManager().ClearTimer(FinishTurnAroundTimerHandle);
}

/// <summary>
/// This function is what actually drives the phoenix to move forward in a perpetual motion. 
/// Takes into account if the Phoenix enters Stationary Mode (reduces the Flight Speed), and also takes into account the Boosting speed multiplier. 
/// </summary>
void AC_Phoenix::PerpetualMotion(float DeltaTime)
{
	float FlightSpeed = FlySpeed * PitchMultiplier; // Pitch Multiplier makes flight faster during downward pitch and slower during upward pitch. 
	if (bIsStationaryMode)
	{
		if (bIsBoosting) BoostCompleted();
		FlightSpeed = StationarySpeed;
	}

	const float SpeedMultiplier = bIsBoosting ? BoostSpeedMultiplier : 1.0f;
	AddMovementInput(GetMesh()->GetForwardVector(), FlightSpeed * SpeedMultiplier);
}

/// <summary>
/// Resets the Phoenix's Rotation back to zero if the player stops pitching.
/// </summary>
void AC_Phoenix::ResetMeshRotation(float DeltaTime)
{
	if (bIsPitching || bPerformLanding) return;

	FRotator MeshRotation = GetMesh()->GetRelativeRotation();
	FRotator NewMeshRotation = FMath::RInterpTo(MeshRotation, FRotator::ZeroRotator, DeltaTime, InterpMeshRotationResetSpeed);
	GetMesh()->SetRelativeRotation(NewMeshRotation);
}

/// This function is bound to the Options Controller's OnInvertControlsChanged delegate to update the local invert control variable
void AC_Phoenix::HandleInvertControlsChanged(bool bInvertControls) { bInvertPitchControl = bInvertControls; }

/// <summary>
/// Bind appropriate functions to their specific InputActions
/// </summary>
void AC_Phoenix::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &AC_Phoenix::OnMoveInputCompleted);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Completed, this, &AC_Phoenix::OnLookInputCompleted);

		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Started, this, &AC_Phoenix::BoostStarted);
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Triggered, this, &AC_Phoenix::OnBoostTriggering);
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Completed, this, &AC_Phoenix::BoostCompleted);
		EnhancedInputComponent->BindAction(BoostAction, ETriggerEvent::Canceled, this, &AC_Phoenix::BoostCompleted);

		EnhancedInputComponent->BindAction(StationaryAction, ETriggerEvent::Triggered, this, &AC_Phoenix::EnterStationaryMode);
		EnhancedInputComponent->BindAction(StationaryAction, ETriggerEvent::Completed, this, &AC_Phoenix::ExitStationaryMode);
		EnhancedInputComponent->BindAction(StationaryAction, ETriggerEvent::Canceled, this, &AC_Phoenix::ExitStationaryMode);
	}
}

/// <summary>
/// Handle Movement Player Input (WASD / Left Analog Stick). Responsible for rotating the phoenix. 
/// Actual movement logic is within the PerpetualMotion() function. 
/// </summary>
void AC_Phoenix::Move(const FInputActionValue& Value)
{
	const FVector2D MovementInputValue = Value.Get<FVector2D>();

	// Have camera reset roll if phoenix stopped rolling. 
	if (FMath::IsNearlyZero(MovementInputValue.X, 0.1f))
		RollMovementInput = 0.0f;

	// Have camera reset pitch if phoenix stopped pitching. 
	if (FMath::IsNearlyZero(MovementInputValue.Y, 0.1f))
	{
		bIsPitching = false;
		PitchMultiplier = 1.0f;
		CameraComponent->OnCameraPitchReset.Broadcast();
	}

	if (MovementInputValue.IsNearlyZero()) return;

	// Check for inverted controls
	InvertPitchValue = bInvertPitchControl ? -1.0f : 1.0f;

	// Adjust roll, yaw, and pitch accordingly. 
	MeshRollingYawMovement(MovementInputValue);
	MeshPitchRotationMovement(MovementInputValue);
	
	MoveInput.X = MovementInputValue.X;
	MoveInput.Y = MovementInputValue.Y;
}

/// <summary>
/// Keep reference to the player's roll input value in the domain of [-1, 1].
/// </summary>
void AC_Phoenix::MeshRollingYawMovement(const FVector2D& MovementInputValue)
{
	if (!FMath::IsNearlyZero(MovementInputValue.X, 0.1f))
		RollMovementInput = MovementInputValue.X;
}

/// <summary>
/// Called in Tick function. Roll is based off A/D (or L Analog) input and Mouse (or R Analog) input. 
/// Yaw is directly coupled to the roll. The more roll there is, the faster the turning speed. 
/// Due to the hierarchy of our actor's root component, visual pivot, and camera pivot, there is a NECESSARY FINAL Camera Pivot counter steering step. 
/// The counter steering step matches the yaw of the camera to the yaw of the actor as they both rotate. 
/// </summary>
void AC_Phoenix::HandleYawAndRoll(float DeltaTime)
{
	FRotator ControlRotation = Controller->GetControlRotation();
	FRotator CameraPivotRotation = CameraPivot->GetRelativeRotation();
	FRotator VisualPivotRotation = VisualPivot->GetRelativeRotation();
	const float CameraYawOffset = CameraPivotRotation.Yaw;

	// Handle Rolling, accounting for both A/D input and Mouse input
	RollMovementInput *= MaxRollAngle;
	float CameraRoll = CameraComponent->GetDeltaYaw() / DeltaTime;
	float TotalRollAmount = RollMovementInput + CameraRoll;
	TotalRollAmount = FMath::Clamp(TotalRollAmount, -MaxRollAngle * 1.2f, MaxRollAngle * 1.2f);
	VisualPivotRotation.Roll = FMath::FInterpTo(VisualPivotRotation.Roll, TotalRollAmount, DeltaTime, InterpRollSpeed);
	VisualPivot->SetRelativeRotation(VisualPivotRotation);

	// Set bIsRolling to true or false, needed for Anim Blueprint. 
	bIsRolling = (FMath::Abs(TotalRollAmount) > 1.0f);

	// Normalize the Roll and then multiply it by the Yaw Speed. This will make our Roll amount dictate how fast or slow the phoenix turns. 
	// High Roll = Faster turning. Low Roll = slower turning. 
	const float YawInput = (VisualPivotRotation.Roll / (MaxRollAngle * 1.2f)) * YawMovementSpeed * DeltaTime * (bIsBoosting ? BoostSpeedMultiplier : 1.0f);
	ControlRotation.Yaw = FMath::FInterpTo(ControlRotation.Yaw, ControlRotation.Yaw + YawInput + CameraYawOffset, DeltaTime, InterpYawSpeed);
	Controller->SetControlRotation(ControlRotation);

	// Counter steer camera pivot rotation back to zero as the phoenix controller rotates. 
	CameraPivotRotation.Yaw = FMath::FInterpTo(CameraPivotRotation.Yaw, 0.0f, DeltaTime, InterpYawSpeed);
	CameraPivot->SetRelativeRotation(CameraPivotRotation);
}

/// <summary>
/// Handle how fast the phoenix should rotate its pitch. Takes stalling into account. 
/// </summary>
void AC_Phoenix::MeshPitchRotationMovement(const FVector2D& MovementInputValue)
{
	// If there is Y input:
	if (!FMath::IsNearlyZero(MovementInputValue.Y, 0.1f))
	{
		bIsPitching = true;
		FRotator CurrentMeshRotation = GetMesh()->GetRelativeRotation();

		// Clear any existing timer to set camera pitch to current inputted values - here cuz it was causing conflicts when the camera was resetting pitch while input was still being given for movement
		CameraComponent->ClearTimerForResetToDefaultCameraPitchHandle();

		// This is the regular pitch input with considerations for inversion, deltatime, boosting multiplier, and the regular pitching speed. 
		float PitchInput = MovementInputValue.Y * InvertPitchValue * PitchingSpeed * GetWorld()->GetDeltaSeconds() * (bIsBoosting ? BoostSpeedMultiplier : 1.0f);
		float Altitude = GetActorLocation().Z;

		// Apply stalling to the PitchInput if needed. 
		ApplyStallingFactor(PitchInput, Altitude);
		CheckForMinAltitude(PitchInput, Altitude);

		CurrentMeshRotation.Pitch = FMath::ClampAngle(CurrentMeshRotation.Pitch + PitchInput, MeshPitchAngleRange.MinValue, MeshPitchAngleRange.MaxValue);
		GetMesh()->SetRelativeRotation(CurrentMeshRotation);

		// Map Pitch Multiplier according to current pitch value. 
		PitchMultiplier = FMath::GetMappedRangeValueClamped(FVector2D(MeshPitchAngleRange.MinValue, MeshPitchAngleRange.MaxValue), 
			FVector2D(PitchMultiplierRange.MinValue, PitchMultiplierRange.MaxValue), CurrentMeshRotation.Pitch);

		// Handle Camera Pitch on Movement Input
		CameraComponent->HandleCameraPitchDuringPhoenixPitchMovement(MovementInputValue.Y, InvertPitchValue);
	}
}

/// <summary>
/// Adjust the Phoenix's pitch if near the maximum altitude.
/// </summary>
void AC_Phoenix::ApplyStallingFactor(float& OutPitchInput, float Altitude)
{
	// Map Altitude to a normalized[0, 1] value between StartStallHeight and MaxAltitudeHeight. 0.0f = No Stall, 1.0f = Complete Stall.
	float NormalizedStallingAlpha = FMath::GetMappedRangeValueClamped(FVector2D(StartStallAltitude, ZAltitudeRange.MaxValue), FVector2D(0.0f, 1.0f), Altitude);

	// Take the NormalizedStallingValue and pass it into a sigmoid curve. The inverse will give a value that determines how much authority the stalling effect will have on the Phoenix's pitch.
	// If StallAuthory = 1.0f, then the Phoenix is not affected by stalling. If StallAuthority = 0.0f, then the Phoenix is absolutely affected by stalling and won't be able to pitch upward anymore. 
	float StallAuthority = 1.0f - FMath::InterpEaseInOut(0.0f, 1.0f, NormalizedStallingAlpha, UpwardStallMultiplier);

	// Also use a GravitationalAuthority to encourage downward pitching within the stall range. 
	float GravitationalAuthority = FMath::Lerp(1.0f, DownwardStallMultiplier, NormalizedStallingAlpha);

	// When pitching upward, try to apply stalling authority if the Phoenix is near the Max Altitude ceiling. 
	if (OutPitchInput > 0.0f)
	{
		// Bias the phoenix's pitch to dive near the stall apex. 
		if (StallAuthority < 0.05f)
		{
			OutPitchInput -= NormalizedStallingAlpha * StallApexDiveStrengthBias * GetWorld()->GetDeltaSeconds() * (bIsBoosting ? BoostSpeedMultiplier : 1.0f);
			if (!bIsStalling && !bIsBoosting) PlayCameraShake(StallCameraShakeClass, StallCameraShakeInstance);
			else if (!bIsStalling && bIsBoosting)PlayCameraShake(StallDuringBoostingCameraShakeClass, StallDuringBoostingCameraShakeInstance);
			bIsStalling = true;
		}
		else
		{
			OutPitchInput *= StallAuthority;
			bIsStalling = false;
		}
	}
	else if (OutPitchInput < 0.0f)
	{
		OutPitchInput *= GravitationalAuthority;
		bIsStalling = false;
	}
}

void AC_Phoenix::GroundSphereTrace()
{
	if (!GetWorld()) return;

	FHitResult Hit;

	const FVector Start = GetActorLocation();
	const FVector End = Start - FVector(0.f, 0.f, 100000.f);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GroundSphereTrace), false); // SCENE_QUERY_STAT macro is to make this function visible in Unreal Insights.
	QueryParams.AddIgnoredActor(this);
	QueryParams.bReturnPhysicalMaterial = true;

	bGroundDetected = GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(GroundSphereTraceRadius), QueryParams); 

	// If sphere trace returns true, determine if can land or not.
	if (bGroundDetected)
	{
		DistanceToGround = Hit.Distance;
		GroundAltitude = Hit.ImpactPoint.Z;
		bGroundIsFlat = Hit.ImpactNormal.Z >= 0.7f; // A Z value of 1 means perfectly flat.

		EPhysicalSurface SurfaceType = UGameplayStatics::GetSurfaceType(Hit);

		// SurfaceType4 is Mud. Mud is always placed underneath water. Don't allow the phoenix to land in water.
		if (SurfaceType == EPhysicalSurface::SurfaceType4) 
			bCanLand = false;

		else
		{
			if (bGroundIsFlat) 
				bCanLand = true;
		}
	}

	// If ground is not detected. 
	else
	{
		DistanceToGround = 999999999999.0f;
		GroundAltitude = 0.0f;
		bGroundIsFlat = false;
		bCanLand = false;
	}
}

void AC_Phoenix::CheckForMinAltitude(float& OutPitchInput, float Altitude)
{
	//UE_LOG(LogTemp, Warning, TEXT("Ground not detected!"));
	if (DistanceToGround > ZAltitudeRange.MinValue) return;

	// Get the phoenix's MinAltitude check range. 
	float MinAltitudeValue = GroundAltitude + ZAltitudeRange.MinValue;
	float MaxAltitudeValue = GroundAltitude + ZAltitudeRange.MinValue + StartPitchingUpFromGroundThreshold;

	// Calculate the pitch up authority within the MinAltitude check range. 
	float NormalizedAlpha = FMath::GetMappedRangeValueClamped(FVector2D(MinAltitudeValue, MaxAltitudeValue), FVector2D(0.0f, 1.0f), Altitude);
	float Authority = FMath::InterpEaseInOut(0.0f, 1.0f, NormalizedAlpha, 3.0f);

	if (Authority >= 1.0f) return;

	if (Authority < 0.33f)
		OutPitchInput += NormalizedAlpha * PitchUpStrengthBias * GetWorld()->GetDeltaSeconds() * (bIsBoosting ? BoostSpeedMultiplier : 1.0f);
	else
		OutPitchInput *= Authority;

	// If the ground is detected and the phoenix is pitching downward, null the pitch value. 
	if (OutPitchInput < 0.0f) OutPitchInput = -GetMesh()->GetRelativeRotation().Pitch;
}

/// <summary>
/// When player lets go of movement input, set appropriate flags and then tell the camera to reset the look pitch.
/// </summary>
void AC_Phoenix::OnMoveInputCompleted(const FInputActionValue& Value)
{
	RollMovementInput = 0.0f;
	PitchMultiplier = 1.0f;
	if (bIsPitching)
	{
		bIsPitching = false;
		bIsStalling = false;
		CameraComponent->OnCameraPitchReset.Broadcast();
	}
}

/// <summary>
/// Wrapper function for the Camera Component's Look function. 
/// </summary>
void AC_Phoenix::Look(const FInputActionValue& Value)
{
	CameraComponent->HandleOnCameraLook(Value);
}

/// <summary>
/// Wrapper function for the Camera Component's Look Input Completed function. 
/// </summary>
void AC_Phoenix::OnLookInputCompleted(const FInputActionValue& Value)
{
	CameraComponent->HandleOnCameraLookInputCompleted(Value);
}

/// <summary>
/// When the phoenix starts their acceleration boost. 
/// </summary>
void AC_Phoenix::BoostStarted()
{
	if (bIsStationaryMode) return;
	bIsBoosting = true;
	OnPhoenixBoostStartedDelegate.Broadcast();

	BoostCameraShakeInstance = PlayCameraShake(BoostCameraShakeClass, BoostCameraShakeInstance);

	// PatchWork - TODO: Refactor later - Update camera pitch limits during boost
	TempMinCameraPitch = MeshPitchAngleRange.MinValue;
	TempMinCameraOnMvmtPitch = CameraComponent->CameraPitchOnMovementRange.MinValue;

	MeshPitchAngleRange.MinValue = BoostMinMeshPitch;
	CameraComponent->CameraPitchOnMovementRange.MinValue = BoostMinCameraPitchOnMvmt;
}

/// <summary>
/// When the player lets go of the boosting key. 
/// </summary>
void AC_Phoenix::BoostCompleted()
{
	bIsBoosting = false;
	OnPhoenixBoostCompletedDelegate.Broadcast();

	OnBoostCompleted();
	StopCameraShake(BoostCameraShakeInstance);
	BoostCameraShakeInstance = nullptr;

	// PatchWork - TODO: Refactor later - Reset camera pitch limits during boost
	MeshPitchAngleRange.MinValue = TempMinCameraPitch;
	CameraComponent->CameraPitchOnMovementRange.MinValue = TempMinCameraOnMvmtPitch;
}

/// <summary>
/// When the player tries to transform, initiate the landing. 
/// Check if able to land, and if so then disable player controls, reset the camera to FVector::Zero, start boosting if not already, and then set the bPerformLanding flag to true. 
/// </summary>
void AC_Phoenix::InitiateLanding()
{
	if (!bCanLand) return; // TODO: Play some sort of negative feedback to indicate to the player that they cannot land (like a phoenix shaking its head anim or an SFX). 

	if (AC_PlayerController* PC = Cast<AC_PlayerController>(GetController())) DisableInput(PC);
	CameraComponent->SetResetCameraFlag();
	ResetAllRotations();
	if (!bIsBoosting) BoostStarted();

	bPerformLanding = true;
}

// While boosting, interpolate to the BoostSocketXOffset
void AC_Phoenix::OnBoostTriggering()
{
	if (bIsStationaryMode) return;
	SpringArm->SocketOffset.X = FMath::FInterpTo(SpringArm->SocketOffset.X, BoostSocketXOffset, GetWorld()->GetDeltaSeconds(), InterpBoostSocketSpeed);
}

FVector AC_Phoenix::DirectionToDodge(int32& Sign)
{
	const float DodgeAngle = CalculateDodgeAngle();

	if (DodgeAngle > 0.0f)
	{
		Sign = 1;
		return GetActorRightVector();
	}

	else if (DodgeAngle < 0.0f)
	{
		Sign = -1;
		return GetActorRightVector() * -1.0f;
	}

	else // If Dodge Angle is 0, randomly choose a right or left dodge. 
	{
		if (FMath::RandBool())
		{
			Sign = 1;
			return GetActorRightVector();
		}
		else
		{
			Sign = -1;
			return GetActorRightVector() * -1.0f; 
		}
	}
}

float AC_Phoenix::CalculateDodgeAngle()
{
	const FVector2D Dir = MoveInput.GetSafeNormal();
	const float AngleRadians = FMath::Atan2(Dir.X, Dir.Y);
	return FMath::RadiansToDegrees(AngleRadians); // In range of -180 to +180 degrees
}

/// <summary>
/// Transform back into Ember. 
/// </summary>
void AC_Phoenix::Transform()
{
	ResetAllRotations();

	TransformToEmberCount++;
	if (bIsBoosting) BoostCompleted(); // If boost is held down at same time as transforming, make sure it is set to complete to avoid speed exploits. 
	AC_GameMode::GetCustomGameMode(this)->SwitchForm(EPlayerForms::Ember);
}

void AC_Phoenix::ResetAllRotations()
{
	FRotator CurrentMeshRotation = GetMesh()->GetRelativeRotation();
	FRotator VisualPivotRotation = VisualPivot->GetRelativeRotation();
	CurrentMeshRotation.Pitch = 0.0f;
	VisualPivotRotation.Roll = 0.0f;
	RollMovementInput = 0.0f;
	GetMesh()->SetRelativeRotation(CurrentMeshRotation);
	VisualPivot->SetRelativeRotation(VisualPivotRotation);
	bIsRolling = false;
	bIsPitching = false;
	bIsStalling = false;
	CameraComponent->OnCameraPitchReset.Broadcast();
}

/// <summary>
/// Cleanup delegate bindings, record Phoenix stats when game is quit. 
/// </summary>
void AC_Phoenix::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Unbind from Controller to avoid dangling binding
	if (UC_UIManagerSubsystem* UIManager = GetWorld()->GetGameInstance()->GetSubsystem<UC_UIManagerSubsystem>())
	{
		if (UC_OptionsWidgetController* OptionsCtrl = UIManager->GetRegisteredControllerTyped<UC_OptionsWidgetController>(EUIWidgetId::Options))
			OptionsCtrl->OnInvertControlsChanged.RemoveDynamic(this, &AC_Phoenix::HandleInvertControlsChanged);
	}

	if (EndPlayReason == EEndPlayReason::Type::EndPlayInEditor || EndPlayReason == EEndPlayReason::Type::Quit)
	{
		if (UC_GameInstance* GameInstance = Cast<UC_GameInstance>(UGameplayStatics::GetGameInstance(this)))
			GameInstance->SetPhoenixCombatStats(LightAttackCount, HeavyAttackCount, TimeHeavyAttackWasHeld, TimeSpentAsPhoenix, TransformToEmberCount);
	}
}