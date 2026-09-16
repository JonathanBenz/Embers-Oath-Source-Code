// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh

#include "UI/C_OptionsWidgetController.h"
#include "Sound/SoundClass.h"
#include "Game/C_PlayerController.h"
#include "Kismet/GameplayStatics.h"

/// <summary>
/// Initialize initial date values for this Widget Controller. 
/// </summary>
void UC_OptionsWidgetController::BroadcastInitialValues()
{
	OnInvertControlsChanged.Broadcast(bInvertUpDownControls);
	OnInvertCameraControlsChanged.Broadcast(bInvertCameraControls);

	if (AC_PlayerController* PController = Cast<AC_PlayerController>(UGameplayStatics::GetPlayerController(this, 0)))
	{
		CameraPitchSensitivity = PController->GetCameraPitchSensitivity();
		CameraYawSensitivity = PController->GetCameraYawSensitivity();
	}
	else
	{
		CameraPitchSensitivity = 1.0f;
		CameraYawSensitivity = 1.0f;
	}
	
	//Set initial slider vlaues
	if (MusicSoundClass && SFXSoundClass && VoiceSoundClass)
	{
		MusicVolume = MusicSoundClass->Properties.Volume;
		SFXVolume = SFXSoundClass->Properties.Volume;
		VoiceVolume = VoiceSoundClass->Properties.Volume;
	}
}

void UC_OptionsWidgetController::BindCallbacksToDependencies() {}

void UC_OptionsWidgetController::SetInvertUpDownControls(bool bNewValue)
{
	if (bInvertUpDownControls != bNewValue)
	{
		bInvertUpDownControls = bNewValue;
		OnInvertControlsChanged.Broadcast(bInvertUpDownControls);
	}
}

void UC_OptionsWidgetController::SetInvertCameraControls(bool bNewValue)
{
	if (bInvertCameraControls != bNewValue)
	{
		bInvertCameraControls = bNewValue;
		OnInvertCameraControlsChanged.Broadcast(bInvertCameraControls);
	}
}

// Setters for camera sensitivity values and broadcasting changes
void UC_OptionsWidgetController::SetCameraPitchSensitivity(float NewPitchSensitivity)
{
	if (CameraPitchSensitivity != NewPitchSensitivity)
	{
		CameraPitchSensitivity = NewPitchSensitivity;
		OnCameraPitchSensitivityChanged.Broadcast(CameraPitchSensitivity);
	}
}

void UC_OptionsWidgetController::SetCameraYawSensitivity(float NewYawSensitivity)
{
	if (CameraYawSensitivity != NewYawSensitivity)
	{
		CameraYawSensitivity = NewYawSensitivity;
		OnCameraYawSensitivityChanged.Broadcast(CameraYawSensitivity);
	}
}

void UC_OptionsWidgetController::SetMusicSensitivity(float NewMusicVolume)
{
	if (MusicVolume != NewMusicVolume)
	{
		MusicVolume = NewMusicVolume;
		if(MusicSoundClass) MusicSoundClass->Properties.Volume = MusicVolume;
	}
}

void UC_OptionsWidgetController::SetSFXSensitivity(float NewSFXVolume)
{
	if (SFXVolume != NewSFXVolume)
	{
		SFXVolume = NewSFXVolume;
		if (SFXSoundClass) SFXSoundClass->Properties.Volume = SFXVolume;
	}
}

void UC_OptionsWidgetController::SetVoiceSensitivity(float NewVoiceVolume)
{
	if (VoiceVolume != NewVoiceVolume)
	{
		VoiceVolume = NewVoiceVolume;
		if (VoiceSoundClass) VoiceSoundClass->Properties.Volume = VoiceVolume;
	}
}