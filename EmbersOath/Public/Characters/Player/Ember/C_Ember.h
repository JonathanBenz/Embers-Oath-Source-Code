// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Hritish Duvvur, Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "Characters/C_BasePlayer.h"
#include "ALS/C_ALSEnums.h"
#include "C_Ember.generated.h"

class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class AC_Item;
class APS_PlayerState;
class UGameplayEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEmberLightAttackPerformed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEmberHeavyAttackPerformed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEmberDodgePerformed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEmberTransformPerformed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FToShowEmberSprintOnboarding);

UCLASS()
class EMBERSOATH_API AC_Ember : public AC_BasePlayer
{
	GENERATED_BODY()
	
public:
	AC_Ember();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetOverlappingItem(TObjectPtr<AC_Item> Item);
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Move(const FInputActionValue& Value) override;
	virtual void Look(const FInputActionValue& Value) override;

	// Called in the GameplayAbility GA_TransformToPhoenix.
	UFUNCTION(BlueprintCallable)
	virtual void Transform() override;

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();

	UFUNCTION(BlueprintCallable)
	void ShowWeapon();

	UFUNCTION(BlueprintCallable)
	void HideWeapon();

	UFUNCTION(BlueprintCallable, Category = "PlayerForms")
	void HideAllAttachedItems();

	UFUNCTION(BlueprintCallable, Category = "PlayerForms")
	void ShowAllAttachedItems();
	
	UFUNCTION(BlueprintCallable)
	float CalculateDeltaYawFromCamera();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void EnableAllAbilities() { bCanTransform = true; bCanLightAttack = true; bCanHeavyAttack = true; bCanDodge = true; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void DisableAllAbilities() { bCanTransform = false; bCanLightAttack = false; bCanHeavyAttack = false; bCanDodge = false; }
	
public:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> EmberInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS | Effects")
	TSubclassOf<UGameplayEffect> OnTransformedEffectToBeApplied;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transformation")
	bool bCanTransform = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool bCanLightAttack = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool bCanHeavyAttack = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dodge")
	bool bCanDodge = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack")
	bool bIsAttacking = false;

	UPROPERTY(VisibleAnywhere, Category = "Analytics")
	int LightAttackCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Analytics")
	int HeavyAttackCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Analytics")
	int TransformToPhoenixCount = 0;

	UPROPERTY(VisibleAnywhere, Category = "Analytics")
	float TimeSpentAsEmber = 0.0f;

	/* Onboarding Events */
	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "OnboardingEvents")
	FOnEmberLightAttackPerformed OnEmberLightAttackPerformed;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "OnboardingEvents")
	FOnEmberHeavyAttackPerformed OnEmberHeavyAttackPerformed;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "OnboardingEvents")
	FOnEmberDodgePerformed OnEmberDodgePerformed;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "OnboardingEvents")
	FOnEmberTransformPerformed OnEmberTransformPerformed;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "OnboardingEvents")
	FToShowEmberSprintOnboarding ToShowEmberSprintOnboarding;

protected:
	virtual void Die_Implementation() override;

	UFUNCTION(BlueprintCallable)
	void TurnAroundAtWorldBounds(const FVector& TurnDir);

	/* Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;
	
	//Magic Circle Camera
	virtual void StartFireRainCamera_Implementation() override;
	virtual void EndFireRainCamera_Implementation() override;

	UPROPERTY(EditAnywhere, Category = "Spells | Fire Rain")
	float FireRainArmLength = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Spells | Fire Rain")
	FRotator FireRainRotation = FRotator(-10.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "Spells | Fire Rain")
	float SpellCamInterpSpeed = 3.0f;

	UPROPERTY(EditAnywhere)
	float AttackInputAxisModifyValue = 0.75;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCurrentlyTargetLocking = false;

	bool bIsCastingFireRain = false;
	float DefaultArmLength = 300.0f;

private: // Functions
	UFUNCTION(BlueprintCallable, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool CanSheatheWeapon();
	
	UFUNCTION(BlueprintCallable, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	void SheatheWeapon();

	UFUNCTION(BlueprintCallable, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool CanDrawWeapon();

	UFUNCTION(BlueprintCallable, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	void DrawWeapon();

	UFUNCTION(BlueprintCallable)
	FName GetDodgeDirectionSectionName();

	float CalculateDodgeAngle();
	void Equip();
	void UpdateTurnAround();
	void FinishTurnAround();
	void PlayEquipMontage(const FName& SectionName);

	FORCEINLINE void OnMoveEnded() { bIsMoving = false; }

	void CheckIfShouldInterpToCombatFOV(float DeltaTime);

private: // Variables
	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<AC_Item> OverlappingItem;

	/* Animation Montages */
	UPROPERTY(EditDefaultsOnly, Category = "Animation Montages")
	TObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Animation Montages")
	TObjectPtr<UAnimMontage> ArmedHitReactMontage;

	/* Input Actions */
	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> EquipAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	FVector2D MoveInput;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	ECurrentGate CurrentGate = ECurrentGate::ECG_Jog;

	bool bIsMoving; // bool which determines if player should roll or backstep.
	
	// vars which make Ember turn around back into the world if she passes through the world bounds. 
	bool bIsMovingForward = false;
	FVector TurnAroundVector;
	FRotator TurnAroundRotation;
	FTimerHandle TurnAroundTimerHandle;
	FTimerHandle FinishTurnAroundTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category = "Camera Shakes")
	TSubclassOf<UCameraShakeBase> SprintCameraShakeClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera Shakes", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraShakeBase> SprintCameraShakeInstance;
	
	float PreviousCamYaw = 0.0f; 
	float CurrentCamYaw = 0.0f;

	// FOV to interpolate toward when in combat. 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FOV Params", meta = (AllowPrivateAccess = "true"))
	float CombatFOV = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FOV Params", meta = (AllowPrivateAccess = "true"))
	float CombatFOVInterpSpeed = 3.0f;

	APS_PlayerState* CachedPlayerState;
};