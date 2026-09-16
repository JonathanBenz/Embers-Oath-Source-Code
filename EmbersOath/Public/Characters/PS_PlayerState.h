// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz, Hritish Duvvur

#pragma once
#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "GameplayAbilitySystem/ASC_AbilitySystemComponent.h"
#include "PS_PlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatEntered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatExited);

class UAbilitySystemComponent;
class UAS_BaseAttributeSet;

UCLASS()
class EMBERSOATH_API APS_PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	APS_PlayerState();
	void AddSpellPoints(int32 AmountToBeAddedToSpellPoints);
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void EnterCombatState();
	virtual void ExitCombatState();

	UPROPERTY(BlueprintAssignable)
	FOnCombatEntered OnCombatEntered;

	UPROPERTY(BlueprintAssignable)
	FOnCombatExited OnCombatExited;

	FORCEINLINE UAS_BaseAttributeSet* GetAttributeSet() const { return AttributeSet; }
	FORCEINLINE int32 GetSpellPoints() const { return SpellPoints; }
	FORCEINLINE void SetSpellPoints(int32 NewSpellPoints) { SpellPoints = NewSpellPoints; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsInCombat() { return bIsInCombat; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UASC_AbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UAS_BaseAttributeSet> AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category= "Spell Menu")
	int32 SpellPoints = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsInCombat = false;
};