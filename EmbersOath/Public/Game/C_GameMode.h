// Copyright (c) 2026, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur, Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Game/C_Enums.h"
#include "Game/C_Structs.h"
#include "C_GameMode.generated.h"

class UDA_AbilityInfo;
class AC_Ember;
class AC_Phoenix;
class USoundClass;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FIsPhoenixCharActive, bool, bIsPhoenixCharActive);

UCLASS()
class EMBERSOATH_API AC_GameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	void SwitchForm(EPlayerForms NewForm);

	FIsPhoenixCharActive OnIsPhoenixCharActive;

	UFUNCTION(BlueprintPure, Category = "Utility")
	static AC_GameMode* GetCustomGameMode(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "CheckpointSystem")
	FPlayerGameAbilitiesSavedState GetPlayerGameAbilitiesSavedState(bool bIsFromRestart);

	UFUNCTION(BlueprintCallable, Category = "CheckpointSystem")
	void LoadPlayerGameAbilitiesSavedState(const FPlayerGameAbilitiesSavedState& SavedState);

	FORCEINLINE EPlayerForms GetCurrentPlayerForm() { return CurrentPlayerForm; }
	FORCEINLINE AC_Ember* GetEmberPawn() { return EmberPawn; }
	FORCEINLINE AC_Phoenix* GetPhoenixPawn() { return PhoenixPawn; }
	
public:
	UPROPERTY(EditDefaultsOnly, Category= "Ability Information")
	TObjectPtr<UDA_AbilityInfo> AbilityInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlayerForm")
	EPlayerForms CurrentPlayerForm = EPlayerForms::Ember;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerForm")
	AC_Ember* EmberPawn;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PlayerForm")
	AC_Phoenix* PhoenixPawn;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SoundClasses")
	TObjectPtr<USoundClass> PhoenixSoundClass;

private:
	void SpawnEmberOnGround();
	void RestorePlayerSpellAbilities();

	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

	UPROPERTY(EditAnywhere, Category = "Spawning", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APawn> EmberBP;

	UPROPERTY(EditAnywhere, Category = "Spawning", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APawn> PhoenixBP;
};