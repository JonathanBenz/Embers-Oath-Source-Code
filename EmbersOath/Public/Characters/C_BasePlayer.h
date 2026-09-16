// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Hritish Duvvur, Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "Characters/C_BaseCharacter.h"
#include "Characters/C_CharacterTypes.h"
#include "GameFramework/Character.h"
#include "Player/PlayerInterface.h"
#include "C_BasePlayer.generated.h"

class UASC_AbilitySystemComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class UPhysicalMaterial;
class AC_Item;
class AC_PlayerController;
class UC_Overlay;

UCLASS()
class EMBERSOATH_API AC_BasePlayer : public AC_BaseCharacter, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AC_BasePlayer();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/* Getters and Setters */
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool IsPlayerDead() { return !IsAlive(); }

	UFUNCTION(BlueprintCallable) 
	FORCEINLINE FVector GetPlayerLocation() const { return PlayerLocation; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetPlayerCameraYaw() const { return PlayerCameraYaw; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UPhysicalMaterial* GetGroundPhysicalMaterial() const { return GroundPhysMaterial.Get(); }
	
	//GAS Functions
	virtual void PossessedBy(AController* NewController) override;
	virtual void InitializeAttributes();
	
	//Patchwork Function
	UFUNCTION(BlueprintCallable)
	void PlayerLevelUp(bool bShouldAddSpellPoints);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void RewardXP(float XPToReward) { XP += XPToReward; }
	
	//Magic Circle Functions
	virtual void ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial = nullptr) override;
	virtual FVector GetMagicCircleLocation_Implementation() override;
	virtual void HideMagicCircle_Implementation() override;

	int32 PlayerLevel = 1;

protected:
	virtual void BeginPlay() override;
	virtual void Die_Implementation() override;

	/* Input Callbacks */
	virtual void Move(const FInputActionValue& Value) PURE_VIRTUAL(AC_BasePlayer::Move); // Define Abstract Function In Child Class
	virtual void Look(const FInputActionValue& Value) PURE_VIRTUAL(AC_BasePlayer::Look); // Define Abstract Function In Child Class
	virtual void Transform() PURE_VIRTUAL(AC_BasePlayer::Transform);					 // Define Abstract Function In Child Class
	
	UFUNCTION(BlueprintCallable)
	UCameraShakeBase* PlayCameraShake(TSubclassOf<UCameraShakeBase> CameraShakeBlueprint, UCameraShakeBase* CameraShakeInstance);

	UFUNCTION(BlueprintCallable)
	void StopCameraShake(UCameraShakeBase* CameraShakeInstance);

	/* Enhanced Input Actions */
	void EnableEnhancedInputMapping(TObjectPtr<UInputMappingContext> InputMappingContext);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LightAttackAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> HeavyAttackAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> TransformAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float XP = 0.0f;

	/* Enum States */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	ECharacterState CharacterState = ECharacterState::ECS_NoWeaponEquipped;

	FVector PlayerLocation; // Player Location for switching forms and keeping location consistent
	float PlayerCameraYaw; // Player Camera Yaw used for the direction bar 

private:
	void InitializeHUDOverlay();
	void RespawnAfterDeath();
	void TrackGroundPhysicalMaterial();
	
	virtual void InitAbilityActorInfo() override;

	// Cached value of the physical material that the player is standing on (if any)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TWeakObjectPtr<UPhysicalMaterial> GroundPhysMaterial = nullptr;
	FTimerHandle PhysicalMaterialGroundTraceTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ember Death", meta = (AllowPrivateAccess = "true"))
	float TimeUntilMenuLoad = 3.0f;
	FTimerHandle EmberDeathTimerHandle;

	int32 SpellPoints = 0;
};