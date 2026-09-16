// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur

#include "Items/C_MagicCircle.h"
#include "Components/DecalComponent.h"

AC_MagicCircle::AC_MagicCircle()
{
	PrimaryActorTick.bCanEverTick = true;
	
	MagicCircleDecalComponent = CreateDefaultSubobject<UDecalComponent>(TEXT("MagicCircleDecal"));
	MagicCircleDecalComponent->SetupAttachment(GetRootComponent());
}