// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur

#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

class EMBERSOATH_API IPlayerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	FVector MagicCircleLocation;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowMagicCircle(UMaterialInterface* DecalMaterial = nullptr);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetMagicCircleLocation();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HideMagicCircle();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void StartFireRainCamera();
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void EndFireRainCamera();
};