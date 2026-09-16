// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "Characters/C_BasePlayer.h"
#include "Game/C_Structs.h"
#include "C_Phoenix.generated.h"

class UC_PhoenixCameraComponent;
class USpringArmComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPhoenixLightAttackPerformed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPhoenixBoostStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPhoenixDodgePerformed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPhoenixBoostCompleted);

UCLASS()
class EMBERSOATH_API AC_Phoenix : public AC_BasePlayer
{
	GENERATED_BODY()

public: // Functions
	AC_Phoenix();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Move(const FInputActionValue& Value) override;
	virtual void Look(const FInputActionValue& Value) override;

	UFUNCTION(BlueprintCallable)
	FVector DirectionToDodge(int32& Sign);
	float CalculateDodgeAngle();

	// Called in the Gameplay Ability GA_TransformToEmber.
	UFUNCTION(BlueprintCallable)
	virtual void Transform() override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void OnBoostCompleted();

	FORCEINLINE bool GetIsPhoenixStalling() { return bIsStalling; }

public: // Variables
	// This variable is set when onboarding happens. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
	bool bCanTransform = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	bool bCanDodge = false;

	// This variable is set to false when playing as Ember, set to true when playing as Phoenix.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerpetualMovement")
	bool bCanMovePerpetual = false;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> PhoenixInputMappingContext;

#pragma region **Attack Bool Checks**
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool bCanLightAttack = false;  
#pragma endregion

#pragma	region **Components**
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UC_PhoenixCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pivots")
	TObjectPtr<USceneComponent> VisualPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pivots")
	TObjectPtr<USceneComponent> CameraPivot;
#pragma endregion

#pragma region **Analytics**
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Analytics")
	int32 LightAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Analytics")
	int32 HeavyAttackCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Analytics")
	int32 TransformToEmberCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Analytics")
	float TimeHeavyAttackWasHeld = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Analytics")
	float TimeSpentAsPhoenix = 0.0f;
#pragma endregion

	// Variable can also be used to determine if camera should pitch by user input
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PerpetualMovement")
	bool bIsPitching = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Spring Arm")
	float DefaultSpringArmLength = 350.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BoostMovement")
	bool bIsBoosting = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Stationary")
	bool bIsStationaryMode;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Altitude|Ground Check")
	bool bCanLand;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Altitude|Ground Check")
	bool bPerformLanding;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "OnboardingEvents")
	FOnPhoenixLightAttackPerformed OnPhoenixLightAttackPerformed;

	UPROPERTY(BlueprintAssignable)
	FOnPhoenixBoostStarted OnPhoenixBoostStartedDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "OnboardingEvents")
	FOnPhoenixBoostCompleted OnPhoenixBoostCompletedDelegate;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "OnboardingEvents")
	FOnPhoenixDodgePerformed OnPhoenixDodgePerformed;

protected:
	UFUNCTION(BlueprintCallable)
	void TurnAround(const FVector& TurnDir);

	UFUNCTION(BlueprintCallable)
	void BoostStarted();

	UFUNCTION(BlueprintCallable)
	void BoostCompleted();

	UFUNCTION(BlueprintCallable)
	void InitiateLanding();

private: // Functions
	void PerpetualMotion(float DeltaTime);
	void ResetMeshRotation(float DeltaTime);
	void HandleYawAndRoll(float DeltaTime);
	void MeshPitchRotationMovement(const FVector2D& MovementInputValue);
	void MeshRollingYawMovement(const FVector2D& MovementInputValue);
	void ResetAllRotations();

	void OnBoostTriggering();

	void OnMoveInputCompleted(const FInputActionValue& Value);
	void OnLookInputCompleted(const FInputActionValue& Value);

	void ApplyStallingFactor(float& OutPitchInput, float Altitude);
	void GroundSphereTrace();
	void CheckForMinAltitude(float& OutPitchInput, float Altitude);
	void UpdateTurnAround();
	void FinishTurnAround();

	FORCEINLINE void EnterStationaryMode() { bIsStationaryMode = true; }
	FORCEINLINE void ExitStationaryMode() { bIsStationaryMode = false; }

	// Delegate handler for GameInstance accessibility change
	UFUNCTION()
	void HandleInvertControlsChanged(bool bInvertControls);

private: // Variables
	// Similar to how FlySpeed is a normalized value. 0.0 = stationary, 0.1 = 10% of the MaxFlySpeed. 
	UPROPERTY(EditAnywhere, Category = "MovementSpeed", meta = (ClampMin = "0.0", ClampMax = "0.1", UIMin = "0.0", UIMax = "0.1"))
	float StationarySpeed = 0.045f;

#pragma region **Movement Speed**
	// Normalized speed value. 0.0 = no speed, 1.0 = MaxFlySpeed. 
	UPROPERTY(EditAnywhere, Category = "MovementSpeed", meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float FlySpeed = 0.42f;

	UPROPERTY(EditAnywhere, Category = "MovementSpeed")
	float MaxFlySpeed = 1800.0f;

	UPROPERTY(EditAnywhere, Category = "MovementSpeed")
	float DefaultAccelerationSpeed = 2048.0f;

	UPROPERTY(EditAnywhere, Category = "MovementSpeed")
	float DefaultDecelerationSpeed = 768.0f;
#pragma endregion

#pragma region **Perpetual Movement**
	// Used by the Phoenix's ABP for checking state. 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roll Params", meta = (AllowPrivateAccess = "true"))
	bool bIsRolling = false;

	UPROPERTY(VisibleAnywhere, Category = "Roll Params")
	float RollMovementInput;

	UPROPERTY(EditAnywhere, Category = "Roll Params")
	float MaxRollAngle = 60.0f;

	UPROPERTY(EditAnywhere, Category = "Pitch Params")
	FMinMaxValues MeshPitchAngleRange = FMinMaxValues(-30.0f, 50.0f);

	UPROPERTY(EditAnywhere, Category = "Roll Params")
	float InterpRollSpeed = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Yaw Params")
	float InterpYawSpeed = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Yaw Params")
	float YawMovementSpeed = 80.0f;

	UPROPERTY(EditAnywhere, Category = "Pitch Params")
	float InterpMeshRotationResetSpeed = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Pitch Params")
	float PitchingSpeed = 100.0f;
#pragma endregion

#pragma region **Altitude** 
	// Min value is relative to the ground's Z-Value, Max value is an absolute value in terms of world height. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Altitude", meta = (AllowPrivateAccess = "true"))
	FMinMaxValues ZAltitudeRange = FMinMaxValues(1000.0f, 10000.0f);

	UPROPERTY(VisibleAnywhere, Category = "Altitude|Ground Check")
	bool bGroundDetected;

	UPROPERTY(VisibleAnywhere, Category = "Altitude|Ground Check")
	bool bGroundIsFlat;

	UPROPERTY(VisibleAnywhere, Category = "Altitude|Ground Check")
	float GroundAltitude = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Altitude|Ground Check", meta = (AllowPrivateAccess = "true"))
	float DistanceToGround = 999999999999.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Altitude|Ground Check", meta = (AllowPrivateAccess = "true"))
	float GroundSphereTraceRadius = 60.0f;

	UPROPERTY(EditAnywhere, Category = "Altitude|Ground Check")
	float StartPitchingUpFromGroundThreshold = 2000.0f;

	UPROPERTY(EditAnywhere, Category = "Altitude|Ground Check")
	float PitchUpStrengthBias = 150.0f;

	// ZAltitudeRange.Max minus this value will dictate when stalling should begin. For example, stalling should start 20m below the max altitude.
	UPROPERTY(EditAnywhere, Category = "Altitude|Stall")
	float StartStallingFromZCeilingThreshold = 3000.0f;

	// Value used for the StallAuthority var in MeshPitchRotationMovement() func. This multiplier dictates how sharp the drop to 0.0f pitch value should be. 
	UPROPERTY(EditAnywhere, Category = "Altitude|Stall")
	float UpwardStallMultiplier = 2.0f;

	// Value used for the GravitationalAuthority var in MeshPitchRotationMovement() func. This multiplier dictates how much times faster the downward pitch should be for reaching the clamped min pitch value. 
	// Make sure this value is never below 1.0f.  
	UPROPERTY(EditAnywhere, Category = "Altitude|Stall")
	float DownwardStallMultiplier = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Altitude|Stall")
	float StallApexDiveStrengthBias = 250.0f;

	// At this height threshold, the Phoenix will start stalling. 
	UPROPERTY(VisibleAnywhere, Category = "Altitude|Stall")
	float StartStallAltitude = ZAltitudeRange.MaxValue - StartStallingFromZCeilingThreshold;

	UPROPERTY(VisibleAnywhere, Category = "Altitude|Stall")
	bool bIsStalling;

	UPROPERTY(VisibleAnywhere, Category = "Altitude|Turn Around")
	FVector TurnAroundVector;

	UPROPERTY(VisibleAnywhere, Category = "Altitude|Turn Around")
	float TurnSteeringDir = 0.0f;

	FTimerHandle TurnAroundTimerHandle;
	FTimerHandle FinishTurnAroundTimerHandle;

#pragma endregion

#pragma region **Crosshair Tracking**
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Crosshair Location", meta = (AllowPrivateAccess = "true"))
	FVector CrosshairWorldLocation;
#pragma endregion

#pragma	region **Invert Controls**
	bool bInvertPitchControl = false;

	UPROPERTY(VisibleAnywhere, Category = "Pitch Params")
	float InvertPitchValue = 1.0f;
#pragma endregion

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> BoostAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> StationaryAction;

	// How much should speed multiply by when boosting. 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoostMovement", meta = (AllowPrivateAccess = "true"))
	float BoostSpeedMultiplier = 2.5f;

	UPROPERTY(VisibleAnywhere, Category = "Pitch Params")
	float PitchMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Pitch Params")
	FMinMaxValues PitchMultiplierRange = FMinMaxValues(100.0f, 0.8f);

	// Temporary variables to store camera pitch values during boost - PatchWork - TODO: Refactor later
	float TempMinCameraPitch = 0.0f;
	float TempMinCameraOnMvmtPitch = 0.0f;

	UPROPERTY(EditAnywhere, Category = "Camera|Spring Arm")
	float BoostSocketXOffset = -100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BoostMovement", meta = (AllowPrivateAccess = "true"))
	float InterpBoostSocketSpeed = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Pitch Params|Boost")
	float BoostMinMeshPitch = -60.f;

	UPROPERTY(EditAnywhere, Category = "Pitch Params|Boost")
	float BoostMinCameraPitchOnMvmt = -60.f;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FVector2D MoveInput;

	// Boost Camera Shake 
	UPROPERTY(EditDefaultsOnly, Category = "Camera Shakes")
	TSubclassOf<UCameraShakeBase> BoostCameraShakeClass;

	// Stall Camera Shake (regular, when not boosting)
	UPROPERTY(EditDefaultsOnly, Category = "Camera Shakes")
	TSubclassOf<UCameraShakeBase> StallCameraShakeClass;

	// Stall Camera Shake (when boosting)
	UPROPERTY(EditDefaultsOnly, Category = "Camera Shakes")
	TSubclassOf<UCameraShakeBase> StallDuringBoostingCameraShakeClass;

	UPROPERTY(VisibleAnywhere, Category = "Camera Shakes")
	TObjectPtr<UCameraShakeBase> BoostCameraShakeInstance;

	UPROPERTY(VisibleAnywhere, Category = "Camera Shakes")
	TObjectPtr<UCameraShakeBase> StallCameraShakeInstance;

	UPROPERTY(VisibleAnywhere, Category = "Camera Shakes")
	TObjectPtr<UCameraShakeBase> StallDuringBoostingCameraShakeInstance;
};