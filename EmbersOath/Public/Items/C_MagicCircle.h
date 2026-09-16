// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Hritish Duvvur

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_MagicCircle.generated.h"

UCLASS()
class EMBERSOATH_API AC_MagicCircle : public AActor
{
	GENERATED_BODY()
	
public:	
	AC_MagicCircle();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UDecalComponent> MagicCircleDecalComponent;
};