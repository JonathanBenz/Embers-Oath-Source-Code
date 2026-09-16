// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Hritish Duvvur

#pragma once
#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AS_BaseAttributeSet.generated.h"

//Built in functions that are being defined. Give us getters and setters
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

UCLASS()
class EMBERSOATH_API UAS_BaseAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public: //Functions
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth) const;

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const;

	UFUNCTION()
	void OnRep_CinderPool(const FGameplayAttributeData& OldCinderPool) const;

	UFUNCTION()
	void OnRep_MaxCinderPool(const FGameplayAttributeData& OldMaxCinderPool) const;

	/* Enemy Attributes */
	UFUNCTION()
	void OnRep_Poise(const FGameplayAttributeData& OldPoise) const;

	UFUNCTION()
	void OnRep_MaxPoise(const FGameplayAttributeData& OldMaxPoise) const;

	UFUNCTION()
	void OnRep_LightAttackDamage(const FGameplayAttributeData& OldLightAttackDamage) const;

	UFUNCTION()
	void OnRep_HeavyAttackDamage(const FGameplayAttributeData& OldHeavyAttackDamage) const;

	UFUNCTION()
	void OnRep_RangedAttackDamage(const FGameplayAttributeData& OldRangedAttackDamage) const;
	/* End Enemy Attributes */

public: //Variables
	
	// Health Attributes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAS_BaseAttributeSet, Health);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAS_BaseAttributeSet, MaxHealth);

	// Spell System, basically Mana Pool
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_CinderPool, Category = "Attributes")
	FGameplayAttributeData CinderPool;
	ATTRIBUTE_ACCESSORS(UAS_BaseAttributeSet, CinderPool);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MaxCinderPool, Category = "Attributes")
	FGameplayAttributeData MaxCinderPool;
	ATTRIBUTE_ACCESSORS(UAS_BaseAttributeSet, MaxCinderPool);

	// Stun/ Poise for enemies. 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Poise, Category = "Attributes| Enemy")
	FGameplayAttributeData Poise;
	ATTRIBUTE_ACCESSORS(UAS_BaseAttributeSet, Poise);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_MaxPoise, Category = "Attributes| Enemy")
	FGameplayAttributeData MaxPoise;
	ATTRIBUTE_ACCESSORS(UAS_BaseAttributeSet, MaxPoise);

	// Damage values for enemies.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_LightAttackDamage, Category = "Attributes| Enemy")
	FGameplayAttributeData LightAttackDamage;
	ATTRIBUTE_ACCESSORS(UAS_BaseAttributeSet, LightAttackDamage);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_HeavyAttackDamage, Category = "Attributes| Enemy")
	FGameplayAttributeData HeavyAttackDamage;
	ATTRIBUTE_ACCESSORS(UAS_BaseAttributeSet, HeavyAttackDamage);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_RangedAttackDamage, Category = "Attributes| Enemy")
	FGameplayAttributeData RangedAttackDamage;
	ATTRIBUTE_ACCESSORS(UAS_BaseAttributeSet, RangedAttackDamage);
};