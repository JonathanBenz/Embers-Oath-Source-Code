// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "HitInterface.h"
#include "Characters/C_CharacterTypes.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include <GameplayEffectTypes.h>
#include "GameplayAbilitySystem/AttributeSets/AS_BaseAttributeSet.h"
#include "C_BaseCharacter.generated.h"

class UAnimMontage;
class AC_Weapon;
class UAbilitySystemComponent;
class UAS_BaseAttributeSet;
class UGameplayEffect;
class UGameplayAbility;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilitiesGiven);

UCLASS()
class EMBERSOATH_API AC_BaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public: // Functions
	AC_BaseCharacter();
	virtual void Tick(float DeltaTime) override;

	/* GAS Functions */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UAS_BaseAttributeSet* GetAttributeSet() const { return PlayerAttributeSet; }
	void GiveDefaultAbilities();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Die();

public: // Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UAbilitySystemComponent> PlayerAbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UAS_BaseAttributeSet> PlayerAttributeSet;

	//This is a gameplay effect that will be applied to the character to initialize their attributes and any GameplayEffect can use this, sets default values for all
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Ability System")
	TSubclassOf<UGameplayEffect> DefaultAttributeEffect;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Ability System")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;
	
protected: // Functions
	virtual void BeginPlay() override;
	virtual bool IsAlive();
	virtual void SpawnDefaultWeapon();
	virtual void EnableRagdoll();
	void DisableCharacterCollision();

	UFUNCTION(BlueprintCallable)
	FName GetDirectionFromHitReaction(const FVector& ImpactPoint);

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollision(ECollisionEnabled::Type CollisionEnabled);

	UFUNCTION(BlueprintCallable)
	FVector GetTranslationWarpTarget();

	UFUNCTION(BlueprintCallable)
	FVector GetRotationWarpTarget();
	
	virtual void InitAbilityActorInfo();

protected: // Variables
	bool bCharacterIsInitialized = false;

	UPROPERTY(VisibleInstanceOnly,BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AC_Weapon> EquippedWeapon;

	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<AC_Weapon> WeaponClass;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Combat")
	TObjectPtr<AActor> CombatTarget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "Combat") // Tracker for the actor hit by C_Weapon
	TObjectPtr<AActor> HitActor;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float WarpTargetDistance = 75.0f;

	/* SFX & VFX */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SFX", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SFX", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USoundBase> DeathSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UParticleSystem> DeathParticles;

	/* Animation Montages */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation Montages")
	TObjectPtr<UAnimMontage> LightAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation Montages")
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation Montages")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation Montages")
	TObjectPtr<UAnimMontage> DodgeMontage;

private: // Variables
	UPROPERTY(EditDefaultsOnly, Category = "Death Params")
	float DespawnTime = 6.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Death Params")
	bool bRagdollOnDeath = true;
};