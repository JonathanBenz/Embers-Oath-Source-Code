// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#pragma once
#include "CoreMinimal.h"
#include "UI/C_BaseWidgetController.h"
#include "Game/C_Enums.h"
#include "C_OptionsWidgetController.generated.h"

class UC_MainMenuUserWidget;
class USoundClass;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInvertControlsChanged, bool, bIsChecked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInvertCameraControlsChanged, bool, bIsChecked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraPitchSensitivityChanged, float, PitchSensitivity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCameraYawSensitivityChanged, float, YawSensitivity);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMusicVolumeChanged, float, MusicVolume);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSFXVolumeChanged, float, SFXVolume);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnVoiceVolumeChanged, float, VoiceVolume);


UCLASS(BlueprintType, Blueprintable)
class EMBERSOATH_API UC_OptionsWidgetController : public UC_BaseWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	// Toggle entrypoint from UI
	UFUNCTION(BlueprintCallable, Category = "Options")
	void SetInvertUpDownControls(bool bNewValue);

	UFUNCTION(BlueprintCallable, Category = "Options")
	void SetInvertCameraControls(bool bNewValue);

	// let gameplay query this via controller
	UFUNCTION(BlueprintPure, Category = "Options")
	bool GetInvertUpDownControls() const { return bInvertUpDownControls; }

	UFUNCTION(BlueprintPure, Category = "Options")
	bool GetInvertCameraControls() const { return bInvertCameraControls; }

	UFUNCTION(BlueprintCallable, Category = "Options")
	float GetCameraPitchSensitivity() const { return CameraPitchSensitivity; }

	UFUNCTION(BlueprintCallable, Category = "Options")
	float GetCameraYawSensitivity() const { return CameraYawSensitivity; }

	UFUNCTION(BlueprintCallable, Category = "Options")
	void SetCameraPitchSensitivity(float NewPitchSensitivity);

	UFUNCTION(BlueprintCallable, Category = "Options")
	void SetCameraYawSensitivity(float NewYawSensitivity);

	//Audio Options
	UFUNCTION(BlueprintCallable, Category = "AudioOptions")
	float GetMusicSensitivity() const { return MusicVolume; }

	UFUNCTION(BlueprintCallable, Category = "AudioOptions")
	float GetSFXSensitivity() const { return SFXVolume; }

	UFUNCTION(BlueprintCallable, Category = "AudioOptions")
	float GetVoiceSensitivity() const { return VoiceVolume; }

	UFUNCTION(BlueprintCallable, Category = "AudioOptions")
	void SetMusicSensitivity(float NewMusicVolume);

	UFUNCTION(BlueprintCallable, Category = "AudioOptions")
	void SetSFXSensitivity(float NewSFXVolume);

	UFUNCTION(BlueprintCallable, Category = "AudioOptions")
	void SetVoiceSensitivity(float NewVoiceVolume);

public: //Variables

	// Event the View binds to
	UPROPERTY(BlueprintAssignable, Category = "OptionsUI")
	FOnInvertControlsChanged OnInvertControlsChanged;

	UPROPERTY(BlueprintAssignable, Category = "OptionsUI")
	FOnInvertCameraControlsChanged OnInvertCameraControlsChanged;

	UPROPERTY(BlueprintAssignable, Category = "OptionsUI")
	FOnCameraPitchSensitivityChanged OnCameraPitchSensitivityChanged;

	UPROPERTY(BlueprintAssignable, Category = "OptionsUI")
	FOnCameraYawSensitivityChanged OnCameraYawSensitivityChanged;

	UPROPERTY(BlueprintAssignable, Category = "AudioOptions")
	FOnMusicVolumeChanged OnMusicVolumeChanged;

	UPROPERTY(BlueprintAssignable, Category = "AudioOptions")
	FOnSFXVolumeChanged OnSFXVolumeChanged;

	UPROPERTY(BlueprintAssignable, Category = "AudioOptions")
	FOnVoiceVolumeChanged OnVoiceVolumeChanged;

	// Variable to see where the options menu was opened from
	UPROPERTY(BlueprintReadWrite, Category = "Options")
	EUIWidgetId WidgetOpenedFrom = EUIWidgetId::PauseMenu;

	UPROPERTY(BlueprintReadWrite, Category = "MainMenuWidget", meta = (AllowPrivateAccess = "true"))
	UC_MainMenuUserWidget* MainMenuWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioOptions")
	TObjectPtr<USoundClass> MusicSoundClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioOptions")
	TObjectPtr<USoundClass> SFXSoundClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AudioOptions")
	TObjectPtr<USoundClass> VoiceSoundClass;

private:
	// By default inverted
	bool bInvertUpDownControls = true; 
	bool bInvertCameraControls = true; 

	float CameraPitchSensitivity = 1.0f;
	float CameraYawSensitivity = 1.0f;

	float MusicVolume = 1.0f;
	float SFXVolume = 1.0f;
	float VoiceVolume = 1.0f;
};