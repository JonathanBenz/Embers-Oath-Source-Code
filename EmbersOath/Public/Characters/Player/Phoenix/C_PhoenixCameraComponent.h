// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Game/C_Structs.h"
#include "InputActionValue.h"
#include "C_PhoenixCameraComponent.generated.h"

class AC_Phoenix;	
class USpringArmComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCameraPitchReset);

UCLASS()
class EMBERSOATH_API UC_PhoenixCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public: //Functions
	UC_PhoenixCameraComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void HandleOnCameraLook(const FInputActionValue& Value);
	void HandleOnCameraLookInputCompleted(const FInputActionValue& Value);
	void HandleCameraPitchDuringPhoenixPitchMovement(float MovementInputValue, float InvertMovementUpDownValue);

	// Implemented in Blueprint as needs timer for the interp to work correctly
	UFUNCTION(BlueprintImplementableEvent)
	void OnCameraPitchReset_Implementation();
	void ClearTimerForResetToDefaultCameraPitchHandle(); // Clears the timer handle used by the blueprint event/ function

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetDeltaYaw() { return DeltaCameraYaw * YawSensitivity; }

	// Interp to BoostFOV when boosting
	UFUNCTION(BlueprintCallable)
	void CheckIfShouldInterpToBoostFOV(float DeltaTime);

	// Interp to StationaryFOV when stationary
	UFUNCTION(BlueprintCallable)
	void CheckIfShouldInterpToStationaryFOV(float DeltaTime);

	// Interp back to default FOV
	UFUNCTION(BlueprintCallable)
	void CheckIfShouldResetToDefaultFOV(float DeltaTime);

	// Setter for InvertCameraControls
	UFUNCTION()
	void SetInvertCameraControls(bool bInvert) { bInvertLookPitchControl = bInvert; }

	FORCEINLINE void SetResetCameraFlag() { bShouldResetCamera = true; }

public: //Variables
	// Delegate for resetting the camera pitch. 
	FOnCameraPitchReset OnCameraPitchReset;

	// The FRotator target value when resetting the camera. 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "CameraRotation")
	FRotator IdealCameraRotation = FRotator::ZeroRotator;

	// Pitch Clamp Values for the camera when it re-orients automatically (no look input) to a pitching phoenix. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CameraPitchOnMovement")
	FMinMaxValues CameraPitchOnMovementRange = FMinMaxValues(-25.0f, 45.0f);

	// Clamped Camera Yaw MinMax Values. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Clamp Ranges")
	FMinMaxValues CameraYawRange = FMinMaxValues(-90.0f, 90.0f);

private: //Functions
	void CheckIfShouldResetCameraRotation(float DeltaTime);
	void ResetCameraIfStalling(FRotator& CurrentCameraRotation, float DeltaTime);
	void CheckIfShouldResetSpringArmLength(float DeltaTime);
	void SetCameraYaw(float YawInput);
	void SetCameraPitch(float PitchInput);
	void CalculateDeltaYawFromCamera();

private: //Variables
	bool bInitialized = false;
	bool bCameraInputDetected = false;
	FTimerHandle CheckForCameraInputHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Speed", meta = (AllowPrivateAccess = "true"))
	float CameraYawSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input Speed", meta = (AllowPrivateAccess = "true"))
	float CameraPitchSpeed = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pitch Params", meta = (AllowPrivateAccess = "true"))
	float IdealCameraPitchWhenLanding = -75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Yaw", meta = (AllowPrivateAccess = "true"))
	float YawSensitivity = 0.1f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Yaw", meta = (AllowPrivateAccess = "true"))
	float DeltaCameraYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Yaw", meta = (AllowPrivateAccess = "true"))
	float PreviousCameraYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Yaw", meta = (AllowPrivateAccess = "true"))
	float CurrentCameraYaw;

	#pragma	region **References**
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Phoenix Parent", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<AC_Phoenix> PhoenixPawn;

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Phoenix Parent", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USpringArmComponent> PhoenixSpringArm;

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Phoenix Parent", meta = (AllowPrivateAccess = "true"))
		TObjectPtr<USceneComponent> CameraPivot;
	#pragma endregion

	#pragma region **Perpetual Camera Movement** 	
		// Clamped Camera Pitch MinMax Values when Phoenix is not also pitching. I.e., when Phoenix is moving in flat in the XY-Plane. 
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Clamp Ranges", meta = (AllowPrivateAccess = "true"))
		FMinMaxValues CameraPitchRange = FMinMaxValues(-60.0f, 90.0f);

		// Interpolation speed for resetting the camera back to the ideal camera rotation.
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerpetualMovement Camera", meta = (AllowPrivateAccess = "true"))
		float InterpCameraResetSpeed = 1.5f;

		// Amount of seconds that passes since input was last detected to reset the camera back to the ideal camera rotation. 
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerpetualMovement Camera", meta = (AllowPrivateAccess = "true"))
		float TimeUntilCameraReset = 3.0f;

		// Amount of tolerance allowed when resetting the camera back to the ideal camera rotation.
		UPROPERTY(EditAnywhere, Category = "Tolerances")
		float ResetRotationTolerance = 4.0f;

		// Flag to check when camera should reset after no input is detected. 
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerpetualMovement Camera", meta = (AllowPrivateAccess = "true"))
		bool bShouldResetCamera = false;
		
		// Flag to check if the phoenix is still pitching after letting go of the mouse/ left analog stick.
		UPROPERTY(EditAnywhere, Category = "PerpetualMovement Camera")
		bool bPhoenixIsStillPitchingWhenLettingGoOfCameraLookInput = false;
	#pragma endregion

	#pragma region **Camera Pitch on Movement**
		// Timer Handle responsible for resetting the camera's pitch.
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pitch Params", meta = (AllowPrivateAccess = "true"))
		FTimerHandle ResetToDefaultCameraPitchHandle;

		// How fast the camera should be pitching.
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pitch Params", meta = (AllowPrivateAccess = "true"))
		float CameraPitchingSpeed = 30.0f;

		// Interpolation speed when resetting the camera pitch back to zero. 
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pitch Params", meta = (AllowPrivateAccess = "true"))
		float InterpCameraPitchResetSpeed = 2.0f;

		// Interpolation speed for camera pitch to match the phoenix's pitch. 
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pitch Params", meta = (AllowPrivateAccess = "true"))
		float InterpCameraPitchSpeed = 10.0f;
	#pragma endregion

	#pragma region **SpringArm For MeshPitch**
		// Amount that the spring arm expands or contracts during pitching. 
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pitch Params", meta = (AllowPrivateAccess = "true"))
		float SpringArmLengthDelta = 100.0f;

		// Interpolation speed when changing springarm length during pitching. 
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pitch Params", meta = (AllowPrivateAccess = "true"))
		float InterpSpringArmChangeSpeed = 2.0f;

		// Flag to check if spring arm length should reset to its default arm length.
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pitch Params", meta = (AllowPrivateAccess = "true"))
		bool bShouldResetSpringArm = false;
	#pragma endregion

	#pragma region **FOV**
		// FOV to interpolate toward when not boosting.
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FOV Params", meta = (AllowPrivateAccess = "true"))
		float DefaultFOV = 90.0f;

		// FOV to interpolate toward when boosting. 
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FOV Params", meta = (AllowPrivateAccess = "true"))
		float BoostFOV = 100.0f;

		// FOV to interpolate toward when boosting. 
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FOV Params", meta = (AllowPrivateAccess = "true"))
		float LandingFOV = 115.0f;

		// FOV to interpolate toward when stationary. 
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FOV Params", meta = (AllowPrivateAccess = "true"))
		float StationaryFOV = 75.0f;

		// How fast the interpolation back to the Default FOV should be. 
		UPROPERTY(EditAnywhere, Category = "FOV Params")
		float InterpDefaultFOVSpeed = 1.0f;

		// How fast the interpolation to the Boost FOV should be. 
		UPROPERTY(EditAnywhere, Category = "FOV Params")
		float InterpBoostFOVSpeed = 3.0f;

		// How fast the interpolation to the Stationary FOV should be. 
		UPROPERTY(EditAnywhere, Category = "FOV Params")
		float InterpStationaryFOVSpeed = 3.0f;
	#pragma endregion

	#pragma	region **Invert Camera Controls**
		bool bInvertLookPitchControl = false;

		UPROPERTY(VisibleAnywhere, Category = "Pitch Params")
		float InvertLookPitchValue = 1.0f;
	#pragma endregion
};