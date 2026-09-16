// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "C_HealthBar.generated.h"

class UProgressBar;

UCLASS()
class EMBERSOATH_API UC_HealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UProgressBar> HealthBar;
};