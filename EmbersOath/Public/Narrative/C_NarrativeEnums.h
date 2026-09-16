// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once

UENUM(BlueprintType)
enum class EInteractionTypes : uint8
{
	EIT_Press UMETA(DisplayName = "Press"),
	ECG_Hold UMETA(DisplayName = "Hold")
};