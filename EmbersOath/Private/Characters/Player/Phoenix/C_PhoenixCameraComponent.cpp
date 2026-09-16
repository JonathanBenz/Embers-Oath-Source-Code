// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Ayushi Parikh

#include "Characters/Player/Phoenix/C_PhoenixCameraComponent.h"
#include "Characters/Player/Phoenix/C_Phoenix.h"
#include "Game/C_PlayerController.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h" 
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include <Engine/World.h>

UC_PhoenixCameraComponent::UC_PhoenixCameraComponent()
{
	bUsePawnControlRotation = false;
}

void UC_PhoenixCameraComponent::BeginPlay()
{
	Super::BeginPlay();
	PhoenixPawn = Cast<AC_Phoenix>(GetOwner());
	check(PhoenixPawn);

	PhoenixSpringArm = PhoenixPawn->SpringArm;
	CameraPivot = PhoenixPawn->CameraPivot;

	bInitialized = true;
	SetFieldOfView(DefaultFOV);
	OnCameraPitchReset.AddDynamic(this, &UC_PhoenixCameraComponent::OnCameraPitchReset_Implementation);
}

void UC_PhoenixCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!bInitialized) return;

	if (PhoenixPawn->bCanMovePerpetual)
	{
		CheckIfShouldResetCameraRotation(DeltaTime);
		CheckIfShouldInterpToBoostFOV(DeltaTime);
		if (PhoenixPawn->bPerformLanding) return;

		CalculateDeltaYawFromCamera();
		CheckIfShouldResetSpringArmLength(DeltaTime);
		CheckIfShouldInterpToStationaryFOV(DeltaTime);
		CheckIfShouldResetToDefaultFOV(DeltaTime);
	}
}

/// <summary>
/// If no camera input has been detected for TimeUntilCameraReset seconds, then attempt to reset the camera to IdealCameraRotation. 
/// </summary>
void UC_PhoenixCameraComponent::CheckIfShouldResetCameraRotation(float DeltaTime)
{
	UWorld* World = GetWorld();
	if (!World) return;

	FRotator CurrentCameraRotation = CameraPivot->GetRelativeRotation();
	ResetCameraIfStalling(CurrentCameraRotation, DeltaTime);

	// Don't reset the camera if aiming within stationary mode. 
	if (PhoenixPawn->bIsStationaryMode)
	{
		World->GetTimerManager().ClearTimer(CheckForCameraInputHandle);
	}

	// Make sure the reset camera timer is turned on after leaving stationary mode, if it isn't already. 
	else
	{
		if (!World->GetTimerManager().IsTimerActive(CheckForCameraInputHandle))
			World->GetTimerManager().SetTimer(CheckForCameraInputHandle, this, &UC_PhoenixCameraComponent::SetResetCameraFlag, TimeUntilCameraReset);
	}

	if (bShouldResetCamera)
	{
		IdealCameraRotation.Yaw = PhoenixPawn->GetMesh()->GetRelativeRotation().Yaw;
		if (PhoenixPawn->bPerformLanding) IdealCameraRotation.Pitch = IdealCameraPitchWhenLanding;

		FRotator NewCameraRotation = FMath::RInterpTo(CurrentCameraRotation, IdealCameraRotation, DeltaTime, InterpCameraResetSpeed);
		CameraPivot->SetRelativeRotation(NewCameraRotation);

		// Check if we are close enough to the ideal rotation to stop rotating,       Or if any pitch input is detected.
		if (CurrentCameraRotation.Equals(IdealCameraRotation, ResetRotationTolerance) || bCameraInputDetected)
		{
			bShouldResetCamera = false;
			bPhoenixIsStillPitchingWhenLettingGoOfCameraLookInput = false;
		}
	}
}

void UC_PhoenixCameraComponent::CalculateDeltaYawFromCamera()
{
	PreviousCameraYaw = CurrentCameraYaw;
	CurrentCameraYaw = CameraPivot->GetRelativeRotation().Yaw;
	DeltaCameraYaw = CurrentCameraYaw - PreviousCameraYaw;
}

void UC_PhoenixCameraComponent::ResetCameraIfStalling(FRotator& CurrentCameraRotation, float DeltaTime)
{
	if (PhoenixPawn->GetIsPhoenixStalling())
	{
		if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(CheckForCameraInputHandle);
		IdealCameraRotation = PhoenixPawn->GetMesh()->GetRelativeRotation();

		FRotator NewCameraRotation = FMath::RInterpTo(CurrentCameraRotation, IdealCameraRotation, DeltaTime, 20.0f);
		CameraPivot->SetRelativeRotation(NewCameraRotation);

		if (CurrentCameraRotation.Equals(IdealCameraRotation, ResetRotationTolerance))
		{
			bShouldResetCamera = false;
			bPhoenixIsStillPitchingWhenLettingGoOfCameraLookInput = false;
		}
	}
}

/// <summary>
/// Interpolate the spring arm back to its default length if necessary. 
/// </summary>
void UC_PhoenixCameraComponent::CheckIfShouldResetSpringArmLength(float DeltaTime)
{
	float CurrentLength = PhoenixSpringArm->TargetArmLength;
	float DefaultLength = PhoenixPawn->DefaultSpringArmLength;

	if (FMath::IsNearlyEqual(CurrentLength, DefaultLength, 5.0f))
		bShouldResetSpringArm = false;

	else if (bShouldResetSpringArm) // NOTE: bShouldResetSpringArm is set inside the OnCameraPitchReset blueprint event. 
		PhoenixSpringArm->TargetArmLength = FMath::FInterpTo(CurrentLength, DefaultLength, DeltaTime, InterpSpringArmChangeSpeed);
}

/// <summary>
/// This function is called anytime mouse or left analog stick input is detected. 
/// </summary>
void UC_PhoenixCameraComponent::HandleOnCameraLook(const FInputActionValue& Value)
{
	const FVector2D CameraInputValue = Value.Get<FVector2D>();
	if (CameraInputValue.GetSafeNormal().Size() < 0.2f) return; // If there is small controller drift or close to no input, don't look. 

	bCameraInputDetected = true;
	SetCameraYaw(CameraInputValue.X);
	SetCameraPitch(CameraInputValue.Y);

	// When a certain amount of time passes after no Look input, reset the camera. 
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CheckForCameraInputHandle);
		World->GetTimerManager().SetTimer(CheckForCameraInputHandle, this, &UC_PhoenixCameraComponent::SetResetCameraFlag, TimeUntilCameraReset);
	}
}

/// <summary>
/// This function is called anytime the mouse or left analog stick was detected in the last frame but not during this current frame. E.g., the input was let go. 
/// </summary>
void UC_PhoenixCameraComponent::HandleOnCameraLookInputCompleted(const FInputActionValue& Value)
{ 
	bCameraInputDetected = false;
	if (PhoenixPawn->bIsPitching) bPhoenixIsStillPitchingWhenLettingGoOfCameraLookInput = true;
}

/// This function is called anytime input yaw from a device is detected (e.g., moving the mouse left or right). 
/// <param name="YawInput"> float YawInput is in the range [-1, +1]. </param>
void UC_PhoenixCameraComponent::SetCameraYaw(float YawInput)
{
	if (UWorld* World = GetWorld())
	{
		if (AC_PlayerController* PController = Cast<AC_PlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			FRotator CurrentCameraRelativeRotation = CameraPivot->GetRelativeRotation();
			const float YawDelta = YawInput * CameraYawSpeed * PController->GetCameraYawSensitivity() * World->DeltaTimeSeconds;
			CurrentCameraRelativeRotation.Yaw = FMath::ClampAngle(CurrentCameraRelativeRotation.Yaw + YawDelta, CameraYawRange.MinValue, CameraYawRange.MaxValue);
			CameraPivot->SetRelativeRotation(CurrentCameraRelativeRotation);
		}
	}
}

/// This function is called anytime input pitch from a device is detected (e.g., moving the mouse up or down). 
/// <param name="PitchInput"> float PitchInput is in the range [-1, +1]. </param>
void UC_PhoenixCameraComponent::SetCameraPitch(float PitchInput)
{
	ClearTimerForResetToDefaultCameraPitchHandle();
	if (UWorld* World = GetWorld())
	{
		if (AC_PlayerController* PController = Cast<AC_PlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
		{
			FRotator CameraRotation = CameraPivot->GetRelativeRotation();
			const float PitchDelta = PitchInput * CameraPitchSpeed * PController->GetCameraPitchSensitivity() * World->DeltaTimeSeconds;
			CameraRotation.Pitch = FMath::ClampAngle(CameraRotation.Pitch - PitchDelta, CameraPitchRange.MinValue, CameraPitchRange.MaxValue);
			CameraPivot->SetRelativeRotation(CameraRotation);
		}
	}
}

/// <summary>
/// Handles how the camera should pitch if the player is pitching the phoenix with WASD/ Left Analog Stick. 
/// Also handles spring arm expansion/ contraction during pitching. 
/// </summary>
void UC_PhoenixCameraComponent::HandleCameraPitchDuringPhoenixPitchMovement(float MovementInputValue, float InvertMovementUpDownValue)
{
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	const float MovementPitchInput = MovementInputValue * InvertMovementUpDownValue;

	// If the phoenix is at the minimum altitude limit but trying to pitch downward, do not match the camera's pitch with the movement input's pitch. Match it with the phoenix flying straight if camera pitch is negative. 
	const bool bPitchedDownButFlyingStraight = MovementPitchInput < 0.0f && PhoenixPawn->GetMesh()->GetRelativeRotation().Pitch >= -0.1;
	if (bPitchedDownButFlyingStraight && !bCameraInputDetected && CameraPivot->GetRelativeRotation().Pitch < 0.0f)
	{ 
		IdealCameraRotation.Pitch = 0.0f;
		SetResetCameraFlag();  
		return; 
	}

	// If the camera has already reset and there is no current look input happening:
	if (!bCameraInputDetected && !bPhoenixIsStillPitchingWhenLettingGoOfCameraLookInput)
	{
		FRotator CurrentRotation = CameraPivot->GetRelativeRotation();
		float AmountToPitchThisFrame = MovementPitchInput * CameraPitchingSpeed * DeltaTime;
		float RawTargetPitch = CurrentRotation.Pitch + AmountToPitchThisFrame;
		float ClampedTargetPitch = FMath::Clamp(RawTargetPitch, CameraPitchOnMovementRange.MinValue, CameraPitchOnMovementRange.MaxValue);

		// If within the clamp values for this specific situation
		if (CurrentRotation.Pitch > CameraPitchOnMovementRange.MinValue && CurrentRotation.Pitch < CameraPitchOnMovementRange.MaxValue)
			CurrentRotation.Pitch = ClampedTargetPitch;

		// Otherwise, the value is within CameraPitchRange but outside of CameraPitchOnMovementRange. Try to bring it into the CameraPitchOnMovementRange range by interpolating to one of the bounds. 
		else
			CurrentRotation.Pitch = FMath::FInterpTo(CurrentRotation.Pitch, ClampedTargetPitch, DeltaTime, InterpCameraPitchSpeed);

		CameraPivot->SetRelativeRotation(CurrentRotation);
		IdealCameraRotation.Pitch = CurrentRotation.Pitch;
	}

	// Expand or contract spring arm when pitching.
	if (MovementInputValue * InvertMovementUpDownValue > 0) // Pitching Up
	{
		InterpCameraPitchResetSpeed = 2.0;
		PhoenixSpringArm->TargetArmLength = FMath::FInterpTo(PhoenixSpringArm->TargetArmLength, PhoenixPawn->DefaultSpringArmLength - SpringArmLengthDelta, DeltaTime, InterpSpringArmChangeSpeed);
	}
	else if (MovementInputValue * InvertMovementUpDownValue < 0) // Pitching Down
	{
		if (bPitchedDownButFlyingStraight) return;
		InterpCameraPitchResetSpeed = 3.0;
		PhoenixSpringArm->TargetArmLength = FMath::FInterpTo(PhoenixSpringArm->TargetArmLength, PhoenixPawn->DefaultSpringArmLength + SpringArmLengthDelta, DeltaTime, InterpSpringArmChangeSpeed);
	}
}

/// <summary>
/// // Clears the timer handle used by the blueprint OnCameraPitchReset_Implementation() event. 
/// </summary>
void UC_PhoenixCameraComponent::ClearTimerForResetToDefaultCameraPitchHandle()
{
	if (UWorld* World = GetWorld()) World->GetTimerManager().ClearTimer(ResetToDefaultCameraPitchHandle);
}

// Boost FOV
void UC_PhoenixCameraComponent::CheckIfShouldInterpToBoostFOV(float DeltaTime)
{
	if (!PhoenixPawn->bIsBoosting) return;

	float NewFOV = -1.0f;

	// Increase boost FOV even more if landing. 
	if (PhoenixPawn->bPerformLanding)
	{
		NewFOV = FMath::FInterpTo(FieldOfView, LandingFOV, DeltaTime, 1.0f);
		if (FMath::IsNearlyEqual(NewFOV, LandingFOV, 0.1f)) NewFOV = LandingFOV;
	}

	// Else, regular boost FOV. 
	else
	{
		NewFOV = FMath::FInterpTo(FieldOfView, BoostFOV, DeltaTime, InterpBoostFOVSpeed);
		if (FMath::IsNearlyEqual(NewFOV, BoostFOV, 0.1f)) NewFOV = BoostFOV;
	}

	SetFieldOfView(NewFOV);
}

// Stationary FOV
void UC_PhoenixCameraComponent::CheckIfShouldInterpToStationaryFOV(float DeltaTime)
{
	if (!PhoenixPawn->bIsStationaryMode) return;
	float NewFOV = FMath::FInterpTo(FieldOfView, StationaryFOV, DeltaTime, InterpStationaryFOVSpeed);
	if (FMath::IsNearlyEqual(NewFOV, StationaryFOV, 0.1f)) NewFOV = StationaryFOV;
	SetFieldOfView(NewFOV);
}

void UC_PhoenixCameraComponent::CheckIfShouldResetToDefaultFOV(float DeltaTime)
{
	// Do not return to default FOV if either in boost or stationary modes. 
	if (PhoenixPawn->bIsBoosting || PhoenixPawn->bIsStationaryMode) return;

	float NewFOV = FMath::FInterpTo(FieldOfView, DefaultFOV, DeltaTime, InterpDefaultFOVSpeed);
	if (FMath::IsNearlyEqual(NewFOV, DefaultFOV, 0.1f)) NewFOV = DefaultFOV;
	SetFieldOfView(NewFOV);
}