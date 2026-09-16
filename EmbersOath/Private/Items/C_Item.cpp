// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "Items/C_Item.h"
#include "Characters/Player/Ember/C_Ember.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

AC_Item::AC_Item()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Item Mesh"));
	ItemMesh->SetGenerateOverlapEvents(false);
	ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetRootComponent(ItemMesh);

	SphereTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere Trigger"));
	SphereTrigger->SetupAttachment(GetRootComponent());

	ItemEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Item FX"));
	ItemEffect->SetupAttachment(GetRootComponent());
}

void AC_Item::BeginPlay()
{
	Super::BeginPlay();
	
	SphereTrigger->OnComponentBeginOverlap.AddDynamic(this, &AC_Item::OnSphereBeginOverlap);
	SphereTrigger->OnComponentEndOverlap.AddDynamic(this, &AC_Item::OnSphereEndOverlap);
}

void AC_Item::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RunningTime += DeltaTime;

	if (ItemState == EItemState::EIS_Hovering)
	{
		Displacement = FVector(0.0f, 0.0f, GetTransformedSin());
		Rotator = FRotator(0.0f, RotationSpeed * RunningTime * DeltaTime, 0.0f);
		AddActorWorldOffset(Displacement);
		AddActorLocalRotation(Rotator);
	}
}

void AC_Item::SpawnPickupSystem()
{
	if (PickupEffect)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PickupEffect, GetActorLocation());
}

void AC_Item::SpawnPickupSound()
{
	if (PickupSound)
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
}

void AC_Item::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (TObjectPtr<AC_Ember> Player = Cast<AC_Ember>(OtherActor))
		Player->SetOverlappingItem(this);
}

void AC_Item::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (TObjectPtr<AC_Ember> Player = Cast<AC_Ember>(OtherActor))
		Player->SetOverlappingItem(nullptr);
}

float AC_Item::GetTransformedSin() const
{
	return Amplitude * FMath::Sin(RunningTime * TimeConstant);
}