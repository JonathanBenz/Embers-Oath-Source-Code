// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "Items/Weapons/C_Weapon.h"
#include "Characters/C_BasePlayer.h"
#include "Characters/Enemies/C_BaseEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Characters/HitInterface.h"
#include "NiagaraComponent.h"
#include "AbilitySystemComponent.h"

AC_Weapon::AC_Weapon()
{
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Collider"));
	WeaponBox->SetupAttachment(GetRootComponent());
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	BoxTraceStart->SetupAttachment(GetRootComponent());

	BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	BoxTraceEnd->SetupAttachment(GetRootComponent());
}

void AC_Weapon::BeginPlay()
{
	Super::BeginPlay();
	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AC_Weapon::OnBoxBeginOverlap);
}

void AC_Weapon::EquipToPlayer(TObjectPtr<USceneComponent> PlayerMesh, const FName& Socket, TObjectPtr<AActor> NewOwner, TObjectPtr<APawn> NewInstigator)
{
	AttachMeshToSocket(PlayerMesh, Socket, NewOwner, NewInstigator);
	UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
}

void AC_Weapon::AttachMeshToSocket(USceneComponent* PlayerMesh, const FName& Socket, AActor* NewOwner, APawn* NewInstigator)
{
	ItemState = EItemState::EIS_Equipped;
	SetOwner(NewOwner);
	SetInstigator(NewInstigator);
	ItemMesh->AttachToComponent(PlayerMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, Socket);
	if (SphereTrigger) SphereTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AC_Weapon::OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ActorIsSameType(OtherActor)) return; // Prevent friendly fire
	ActorOnHit = OtherActor;

	FHitResult BoxHit;
	BoxTrace(BoxHit);
	if (BoxHit.GetActor())
	{
		if (ActorIsSameType(BoxHit.GetActor())) return; // Prevent friendly fire
	}
}

bool AC_Weapon::ActorIsSameType(TObjectPtr<AActor> OtherActor)
{
	const bool EnemyHitEnemy = GetOwner()->ActorHasTag(FName("Enemy")) && OtherActor->ActorHasTag(FName("Enemy"));;
	const bool OwnerHittingSelf = OtherActor == GetOwner();
	return EnemyHitEnemy || OwnerHittingSelf;
}

void AC_Weapon::BoxTrace(FHitResult& BoxHit)
{
	const FVector Start = BoxTraceStart->GetComponentLocation();
	const FVector End = BoxTraceEnd->GetComponentLocation();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.AddUnique(GetOwner());
	for (const auto& Actor : IgnoreActors)
		ActorsToIgnore.AddUnique(Actor);

	UKismetSystemLibrary::BoxTraceSingle
	(
		this, Start, End, BoxTraceExtent, BoxTraceStart->GetComponentRotation(),
		ETraceTypeQuery::TraceTypeQuery1, false, ActorsToIgnore,
		bShowBoxDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		BoxHit, true
	);
}