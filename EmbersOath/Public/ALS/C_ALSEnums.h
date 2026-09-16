// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
UENUM(BlueprintType)
enum class ECurrentGate : uint8
{
	ECG_Jog UMETA(DisplayName = "Jog"),
	ECG_Sprint UMETA(DisplayName = "Sprint")
};