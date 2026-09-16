// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "Characters/C_BaseCharacter.h"
#include "Items/Weapons/C_Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySystem/AttributeSets/AS_BaseAttributeSet.h"
#include <Game/C_GameInstance.h>
#include "GameplayAbilitySystem/ASC_AbilitySystemComponent.h"

AC_BaseCharacter::AC_BaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void AC_BaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	bCharacterIsInitialized = true;
}

void AC_BaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UAbilitySystemComponent* AC_BaseCharacter::GetAbilitySystemComponent() const { return PlayerAbilitySystemComponent; }

void AC_BaseCharacter::GiveDefaultAbilities()
{
	UASC_AbilitySystemComponent* EOASC = Cast<UASC_AbilitySystemComponent>(PlayerAbilitySystemComponent);
	if (!HasAuthority()) return;
	
	EOASC->AddCharacterAbilities(DefaultAbilities);
}

void AC_BaseCharacter::SetWeaponCollision(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon && EquippedWeapon->GetWeaponBox())
	{
		EquippedWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		EquippedWeapon->IgnoreActors.Empty();
	}
}

FVector AC_BaseCharacter::GetTranslationWarpTarget()
{
	if (!CombatTarget) return FVector::Zero();

	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	FVector DirFromTarget = (GetActorLocation() - CombatTargetLocation).GetSafeNormal();
	DirFromTarget *= WarpTargetDistance;
	return CombatTargetLocation + DirFromTarget;
}

FVector AC_BaseCharacter::GetRotationWarpTarget()
{
	if (!CombatTarget) return FVector::Zero();

	return CombatTarget->GetActorLocation();
}

void AC_BaseCharacter::InitAbilityActorInfo() {}

bool AC_BaseCharacter::IsAlive()
{
	return PlayerAttributeSet && PlayerAttributeSet->GetHealth() > 0;
}

/// <summary>
/// Die is currently being in the blueprint GameplayCues of both the player and the enemies. 
/// </summary>
void AC_BaseCharacter::Die_Implementation()
{
	if (Tags.Contains(FName("Dead"))) return; // Exit the function immediately if already dead.
	
	Tags.Add(FName("Dead"));
	UE_LOG(LogTemp, Display, TEXT("%s has died!"), *GetName());
	DisableCharacterCollision();
	if (bRagdollOnDeath) EnableRagdoll();
	SetWeaponCollision(ECollisionEnabled::NoCollision); // Avoid edge case of weapon still dealing damage upon death
	SetLifeSpan(DespawnTime);
}

void AC_BaseCharacter::EnableRagdoll()
{
	const FVector CurrentVelocity = GetCharacterMovement()->Velocity;

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->SetComponentTickEnabled(false);

	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	SetActorEnableCollision(true);
	GetMesh()->SetAllBodiesSimulatePhysics(true);
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();
	GetMesh()->bBlendPhysics = true;

	GetMesh()->SetAllPhysicsLinearVelocity(CurrentVelocity);
}

void AC_BaseCharacter::DisableCharacterCollision()
{
	// Disable collision on death
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
}

void AC_BaseCharacter::SpawnDefaultWeapon()
{
	TObjectPtr<UWorld> World = GetWorld();
	if (World && WeaponClass)
	{
		TObjectPtr<AC_Weapon> DefaultWeapon = World->SpawnActor<AC_Weapon>(WeaponClass);
		DefaultWeapon->AttachMeshToSocket(GetMesh(), FName("WeaponSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

FName AC_BaseCharacter::GetDirectionFromHitReaction(const FVector& ImpactPoint)
{
	const FVector Forward = GetActorForwardVector();
	const FVector ImpactXY(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactXY - GetActorLocation()).GetSafeNormal();

	const double CosTheta = FVector::DotProduct(Forward, ToHit);
	double Theta = FMath::RadiansToDegrees(FMath::Acos(CosTheta));

	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if (CrossProduct.Z < 0)
		Theta *= -1.0f;

	if (Theta < 45 && Theta >= -45)
		return FName("Front");
	else if (Theta < 135 && Theta >= 45)
		return FName("Right");
	else if (Theta < -45 && Theta >= -135)
		return FName("Left");
	else
		return FName("Back");
}