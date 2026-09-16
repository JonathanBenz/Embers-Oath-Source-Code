// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "C_Item.generated.h"

class USphereComponent;
class UNiagaraComponent;
class UNiagaraSystem;

enum class EItemState : uint8
{
	EIS_Hovering,
	EIS_Equipped
};

UCLASS()
class EMBERSOATH_API AC_Item : public AActor
{
	GENERATED_BODY()
	
public:	
	AC_Item();
	virtual void Tick(float DeltaTime) override;

protected: // Functions
	virtual void BeginPlay() override;

	virtual void SpawnPickupSystem();
	virtual void SpawnPickupSound();

	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected: // Variables
	EItemState ItemState = EItemState::EIS_Hovering;

	/* Components */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereTrigger;

	/* VFX and SFX */
	UPROPERTY(EditAnywhere, Category = "VFX")
	TObjectPtr<UNiagaraComponent> ItemEffect;

	UPROPERTY(EditAnywhere, Category = "VFX")
	TObjectPtr<UNiagaraSystem> PickupEffect;

	UPROPERTY(EditAnywhere, Category = "SFX")
	TObjectPtr<USoundBase> PickupSound;

private: // Functions
	float GetTransformedSin() const;

private: // Variables
	FVector Displacement;
	FRotator Rotator;

	UPROPERTY(EditDefaultsOnly, Category = "Hover Params")
	float Amplitude = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Hover Params")
	float TimeConstant = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Hover Params")
	float RotationSpeed = 10.0f;

	UPROPERTY(VisibleAnywhere)
	float RunningTime;
};