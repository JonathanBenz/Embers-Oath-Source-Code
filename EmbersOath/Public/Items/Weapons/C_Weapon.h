// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "Items/C_Item.h"
#include "C_Weapon.generated.h"

class USoundBase;
class UBoxComponent;

UCLASS()
class EMBERSOATH_API AC_Weapon : public AC_Item
{
	GENERATED_BODY()

public: // Functions
	AC_Weapon();
	void EquipToPlayer(TObjectPtr<USceneComponent> PlayerMesh, const FName& Socket, TObjectPtr<AActor> NewOwner, TObjectPtr<APawn> NewInstigator);

	UFUNCTION(BlueprintCallable)
	void AttachMeshToSocket(USceneComponent* PlayerMesh, const FName& Socket, AActor* NewOwner, APawn* NewInstigator);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UBoxComponent* GetWeaponBox() const { return WeaponBox; }

	UPROPERTY(BlueprintReadWrite)
	TArray<TObjectPtr<AActor>> IgnoreActors; // On a hit, add the hit actor to be ignored for the remaining ticks of the attack animation, that way the actor is only hit once on an attack. 

public: // Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon Properties")
	AActor* ActorOnHit;
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<USceneComponent> BoxTraceStart;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<USceneComponent> BoxTraceEnd;

protected:
	virtual void BeginPlay() override;
	bool ActorIsSameType(TObjectPtr<AActor> OtherActor);

	UFUNCTION()
	void OnBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private: // Functions
	void BoxTrace(FHitResult& BoxHit);

private: // Variables
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	FVector BoxTraceExtent = FVector(5.0f, 5.0f, 5.0f);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	bool bShowBoxDebug = false;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon Properties")
	TObjectPtr<USoundBase> EquipSound;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	TObjectPtr<UBoxComponent> WeaponBox;
};