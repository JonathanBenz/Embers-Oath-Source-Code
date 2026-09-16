// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "C_HealthBarComponent.generated.h"

class UC_HealthBar;

UCLASS()
class EMBERSOATH_API UC_HealthBarComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	void SetHealthPercent(float HealthPercent);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UC_HealthBar> HealthBarWidget;
};