// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "Characters/Enemies/C_BaseEnemy.h"
#include "Characters/C_BasePlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Checkpoint/C_CheckpointSubsystem.h"
#include "UI/C_HealthBarComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AI/C_AIController.h"
#include "AI/C_HerdManager.h"
#include "AI/Utility/BTTask_UtilityBase.h"
#include "Game/C_GameMode.h"
#include "Items/Weapons/C_Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayAbilitySystem/ASC_AbilitySystemComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AC_BaseEnemy::AC_BaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	
	PlayerAbilitySystemComponent = CreateDefaultSubobject<UASC_AbilitySystemComponent>("CustomAbilitySystemComponent");
	PlayerAbilitySystemComponent->SetIsReplicated(true);
	PlayerAbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	PlayerAttributeSet = CreateDefaultSubobject<UAS_BaseAttributeSet>("CustomAttributeSet");

	HealthBarWidget = CreateDefaultSubobject<UC_HealthBarComponent>(TEXT("Health Bar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bUseControllerDesiredRotation = true; // Look in the direction of AI Controller focus
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void AC_BaseEnemy::BeginPlay()
{
	Super::BeginPlay();
	InitializeEnemy();
}

void AC_BaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TickCooldowns(DeltaTime);
}

void AC_BaseEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (bCharacterIsInitialized) return; // Do not re-initialize anything GAS related upon re-possesion! 

	InitAbilityActorInfo();
	
	if (HasAuthority())
	{
		for (TSubclassOf<UGameplayAbility> Ability : DefaultAbilities)
			PlayerAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability, 1));
	}
}

void AC_BaseEnemy::InitializeEnemy()
{
	SpawnDefaultWeapon();
	HideHealthBar();
	HealthBarWidget->SetHealthPercent(1.0f);
	EnemyAIController = Cast<AC_AIController>(GetController());
	Blackboard = EnemyAIController->GetBlackboardComponent();
	check(BehaviorTree);
	check(EnemyAIController);
	check(Blackboard);
	check(DefaultAttributeEffect);
	GetWorldTimerManager().SetTimer(WorldKnowledgeHandle, this, &AC_BaseEnemy::UpdateWorldKnowledge, 0.15f, true); // Update the AI Agent's World Knowledge
	Tags.Add(FName("Enemy"));
}

void AC_BaseEnemy::InitAbilityActorInfo()
{
	Super::InitAbilityActorInfo();

	PlayerAbilitySystemComponent = GetAbilitySystemComponent();
	PlayerAbilitySystemComponent->InitAbilityActorInfo(this, this);
	PlayerAttributeSet = GetAttributeSet();
	Cast<UASC_AbilitySystemComponent>(PlayerAbilitySystemComponent)->AbilityActorInfoSet();

	InitializeAttributes();
}

void AC_BaseEnemy::InitializeAttributes()
{
	if (PlayerAbilitySystemComponent && DefaultAttributeEffect)
	{
		FGameplayEffectContextHandle EffectContext = PlayerAbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		FGameplayEffectSpecHandle SpecHandle = PlayerAbilitySystemComponent->MakeOutgoingSpec(DefaultAttributeEffect, 1, EffectContext);

		if (SpecHandle.IsValid()) 
			FActiveGameplayEffectHandle GEHandle = PlayerAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

void AC_BaseEnemy::SetMovementSpeed(EEnemyMovementSpeed MovementSpeed)
{
	switch (MovementSpeed)
	{
	case EEnemyMovementSpeed::EEMS_None:
		GetCharacterMovement()->MaxWalkSpeed = 0.0f;
		break;

	case EEnemyMovementSpeed::EEMS_Walk:
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		break;

	case EEnemyMovementSpeed::EEMS_Run:
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		break;

	case EEnemyMovementSpeed::EEMS_Flee:
		GetCharacterMovement()->MaxWalkSpeed = FleeSpeed;
		break;
	}
}

bool AC_BaseEnemy::CheckIfEnemyHasDiedOnce()
{
	//First check if enemy is already killed, if yes, call die function immediately.
	if (MechBeastIdentityTag.IsValid())
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UC_CheckpointSubsystem* CheckpointSys = GI->GetSubsystem<UC_CheckpointSubsystem>())
			{
				if (CheckpointSys->IsEnemyOrHerdKilled(MechBeastIdentityTag))
				{
					return true;
				}
			}
		}
	}
	return false;
}

/// <summary>
/// Update the enemy's knowledge of the World State. Set its blackboard keys to reflect updated world knowledge. 
/// </summary>
void AC_BaseEnemy::UpdateWorldKnowledge()
{
	if (Player = Cast<AC_BasePlayer>(UGameplayStatics::GetPlayerCharacter(this, 0))) 
	{
		// Update cached values.
		PlayerLocation = Player->GetActorLocation();
		DistanceToPlayer = FVector::Dist(PlayerLocation, GetActorLocation());
		bPlayerIsGrounded = Player->GetGroundPhysicalMaterial() ? true : false;
		bPlayerIsDead = Player->IsPlayerDead();
		bPlayerIsPhoenix = AC_GameMode::GetCustomGameMode(this)->GetCurrentPlayerForm() == EPlayerForms::Phoenix ? true : false;
		FVector DirFromPlayerToEnemy = (GetActorLocation() - Player->GetActorLocation());
		DirFromPlayerToEnemy = FVector(DirFromPlayerToEnemy.X, DirFromPlayerToEnemy.Y, 0.0f).GetSafeNormal();
		bIsInFrontOfPlayer = FVector::DotProduct(Player->GetActorForwardVector(), DirFromPlayerToEnemy) > 0.0f ? true : false;
		bCanSeePlayer = CanSeePlayer() ? true : false;

		// Update blackboard keys
		Blackboard->SetValueAsObject(TEXT("Player"), Player);
		Blackboard->SetValueAsVector(TEXT("PlayerLocation"), PlayerLocation);
		Blackboard->SetValueAsFloat(TEXT("DistanceToPlayer"), DistanceToPlayer);
		Blackboard->SetValueAsBool(TEXT("bPlayerIsGrounded"), bPlayerIsGrounded);
		Blackboard->SetValueAsBool(TEXT("bPlayerIsDead"), bPlayerIsDead);
		Blackboard->SetValueAsBool(TEXT("bPlayerIsPhoenix"), bPlayerIsPhoenix);
		Blackboard->SetValueAsBool(TEXT("bIsInFrontOfPlayer"), bIsInFrontOfPlayer);
		Blackboard->SetValueAsBool(TEXT("bCanSeePlayer"), bCanSeePlayer);
	}
}

/// <summary>
/// When a Utility Task is performed, keep track of its cooldown time. Should be called within the Execution of the Task. 
/// </summary>
void AC_BaseEnemy::StartTaskCooldown(const int32 TaskID, const float CooldownTime)
{
	TaskCooldowns.Add(TaskID, CooldownTime);
}

/// <summary>
/// For each ability within the TaskCooldowns TMap, decrement their cooldowns by DeltaTime. 
/// </summary>
void AC_BaseEnemy::TickCooldowns(float DeltaTime)
{
	for (auto& Pair : TaskCooldowns)
		Pair.Value -= DeltaTime;
}

/// <summary>
/// Play an animation when seeing the player for the first time. 
/// </summary>
void AC_BaseEnemy::PlayPassiveToAttackTransitionAnimation()
{
	TObjectPtr<UAnimInstance> AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && PassiveToAttackTransitionAnimation)
	{
		AnimInstance->Montage_Play(PassiveToAttackTransitionAnimation);
	}
	else if (!AnimInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("No reference to an AnimInstance!"));
	}
	else 
		UE_LOG(LogTemp, Warning, TEXT("PassiveToAttackTransitionAnim was not set in blueprint!"));
}

/// <summary>
/// When the UtilityTask is evaluating, it will check from the enemy whether or not it is currently on cooldown. If so, its evaluation will get skipped. 
/// </summary>
/// <param name="TaskBeingEvaluated"> This function is called by the UtilityTask, so it will pass the This pointer as its argument. </param>
bool AC_BaseEnemy::IsTaskOnCooldown(const int32 TaskID) const
{
	const float* RemainingTime = TaskCooldowns.Find(TaskID);
	if (RemainingTime) 
		return RemainingTime && *RemainingTime > 0.0f;
	
	return false;
}

void AC_BaseEnemy::HideHealthBar()
{
	if (HealthBarWidget) 
	{
		HealthBarWidget->SetVisibility(false);
		bHealthBarVisible = false;
	}
}

void AC_BaseEnemy::ShowHealthBar()
{
	if (HealthBarWidget)
	{
		HealthBarWidget->SetVisibility(true);
		bHealthBarVisible = true;
	}
}

float AC_BaseEnemy::GetCurrentAgentSpeed()
{
	switch (EnemyState)
	{
		case EEnemyState::EES_Patrolling:
			return WalkSpeed;
		case EEnemyState::EES_Attacking:
			return RunSpeed;
		default:
			return 0.0f;
	}
}

bool AC_BaseEnemy::CanSeePlayer()
{
	const FVector EnemyBodyLocation = GetActorLocation() + FVector(0.0f, 0.0f, 20.0f);
	const FVector PlayerBodyLocation = PlayerLocation + FVector(0.0f, 0.0f, 20.0f);

	FHitResult OutHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(UGameplayStatics::GetPlayerCharacter(this, 0));

	// If a hit is detected between the ignored Enemy and ignored Player, then there is an obstacle. Cannot see player. 
	if (GetWorld()->LineTraceSingleByChannel(OutHit, EnemyBodyLocation, PlayerBodyLocation, ECC_Visibility, QueryParams)) 
		return false;
	
	return true;
}

void AC_BaseEnemy::Die_Implementation()
{
	Super::Die_Implementation();
	EnemyState = EEnemyState::EES_Dead;
	bHasPlayedDeathEffect = true; // Death effect is played in the gameplay cue
	HideHealthBar();
	GetWorldTimerManager().ClearTimer(WorldKnowledgeHandle);
	GetCharacterMovement()->bOrientRotationToMovement = false;  // Corpse shouldn't rotate to face the player
	DetachFromControllerPendingDestroy();						// Nullify the AI Controller, this will prevent the Behavior Tree from running when the enemy is dead. 
	PlayerAbilitySystemComponent->CancelAllAbilities();
	if (HerdManagerOwner)
		HerdManagerOwner->DecrementEnemiesLeft(this);

	// If this enemy has a valid MechBeastIdentityTag, mark it as killed in the CheckpointSubsystem so that it doesn't respawn when the player dies and respawns at a checkpoint.
	if (MechBeastIdentityTag.IsValid())
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (UC_CheckpointSubsystem* CheckpointSys = GI->GetSubsystem<UC_CheckpointSubsystem>())
			{
				CheckpointSys->MarkEnemyOrHerdAsKilled(MechBeastIdentityTag);
			}
		}
	}
}

void AC_BaseEnemy::Destroyed()
{
	Super::Destroyed();
	if (EquippedWeapon) EquippedWeapon->Destroy();
}

void AC_BaseEnemy::UpdateHealthbar()
{
	if (HealthBarWidget) 
	{
		if (!bHealthBarVisible && IsAlive()) ShowHealthBar();
		HealthBarWidget->SetHealthPercent(PlayerAttributeSet->GetHealth() / PlayerAttributeSet->GetMaxHealth());
	}
}

void AC_BaseEnemy::InterruptBehaviorTreeLogic(const FString& Reason)
{
	EnemyAIController->StopMovement();
	if (UBehaviorTreeComponent* BTC = Cast<UBehaviorTreeComponent>(EnemyAIController->GetBrainComponent()))
		BTC->PauseLogic(Reason);
}

void AC_BaseEnemy::ResumeBehaviorTreeLogic(const FString& Reason)
{
		if (UBehaviorTreeComponent* BTC = Cast<UBehaviorTreeComponent>(EnemyAIController->GetBrainComponent()))
		BTC->ResumeLogic(Reason);
}