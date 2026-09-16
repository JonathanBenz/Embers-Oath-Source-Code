// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contibutors: Jonathan Benz

#include "UI/C_HealthBarComponent.h"
#include "UI/C_HealthBar.h"
#include "Components/ProgressBar.h"

void UC_HealthBarComponent::SetHealthPercent(float HealthPercent)
{
	if (HealthBarWidget && HealthBarWidget->HealthBar)
		HealthBarWidget->HealthBar->SetPercent(HealthPercent);
}

void UC_HealthBarComponent::BeginPlay()
{
	Super::BeginPlay();
	HealthBarWidget = Cast<UC_HealthBar>(GetUserWidgetObject());
}