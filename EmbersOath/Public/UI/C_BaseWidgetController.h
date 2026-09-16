// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Ayushi Parikh, Hritish Duvvur

#pragma once
#include "CoreMinimal.h"
#include "Game/C_PlayerController.h"
#include "UObject/NoExportTypes.h"
#include "C_BaseWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FEmberAbilityInfo&, Info);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewValue);

class APS_PlayerState;
class UAS_BaseAttributeSet;
class UASC_AbilitySystemComponent;
class UAbilitySystemComponent;
class UAttributeSet;
class AC_PlayerController;
class UC_GameInstance;
class AC_GameMode;
class AC_BasePlayer;
class UDA_AbilityInfo;

/// <summary>
/// This struct will hold references to the player's Player Controller, GameInstance, GameMode, Ability System Component, and Attribute Set when constructed. 
/// Use this as a set of parameters to pass into the Widget Controller so that these data can be used within a widget's blueprints if needed. 
/// </summary>
USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()

	FWidgetControllerParams() {} //Default constructor
	
	FWidgetControllerParams(AC_PlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
		: PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS) {}

	FWidgetControllerParams(AC_PlayerController* PC, UC_GameInstance* GI, AC_GameMode* GM, UAbilitySystemComponent* ASC, UAttributeSet* AS)
		: PlayerController(PC), GameInstance(GI), GameMode(GM), AbilitySystemComponent(ASC), AttributeSet(AS) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AC_PlayerController> PlayerController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UC_GameInstance> GameInstance = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<AC_GameMode> GameMode = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	APlayerState* PlayerState;
};

UCLASS()
class EMBERSOATH_API UC_BaseWidgetController : public UObject
{
	GENERATED_BODY()
	
public:
	// Virtual function intended to be called immediately after the respective Controller is instantiated - used to create any sub-controllers needed for this Widget Controller
	virtual void InitializeSubControllers() {}

	// Set the variables in the protected section to equal the values from the passed in WCParams. 
	virtual void SetWidgetControllerParams(const FWidgetControllerParams& WCParams);

	/* Abstract Methods. Child classes must define these. */
	// Initialize any initial date values for the Widget Controller.
	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues() PURE_VIRTUAL(UC_BaseWidgetController::BroadcastInitialValues);
	
	// Use this method for changed data values to broadcast their new values to any widgets that are listening for it.
	virtual void BindCallbacksToDependencies() PURE_VIRTUAL(UC_BaseWidgetController::BindCallbacksToDependencies);
	/* ----------- End Abstract Methods. ----------------- */
	
	UFUNCTION(BlueprintCallable)
	void BroadcastAbilityInfo();

	/** Returns the PlayerController associated with this widget controller */
	UFUNCTION(BlueprintCallable, Category = "WidgetControllerParams")
	AC_PlayerController* GetPlayerController() const { return PlayerController; }
	
	UPROPERTY(BlueprintAssignable, Category = "GAS|Messages")
	FAbilityInfoSignature AbilityInfoDelegate;

protected:
	bool bIsBoundToDependencies = false; // Set to true when BindCallbacksToDependencies is called to prevent rebinding if the function is called again - as the callbacks should only be bound once and the function is called multiple times when the widget is shown

	UPROPERTY(BlueprintReadOnly, Category = "WidgetControllerParams")
	TObjectPtr<AC_PlayerController> PlayerController;
	
	UPROPERTY(BlueprintReadOnly, Category = "WidgetControllerParams")
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetControllerParams")
	TObjectPtr<UC_GameInstance> GameInstance;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetControllerParams")
	TObjectPtr<AC_GameMode> GameMode;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetControllerParams")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetControllerParams")
	TObjectPtr<UAttributeSet> AttributeSet;
	
	UPROPERTY(BlueprintReadWrite, Category = "WidgetControllerParams")
	TObjectPtr<APS_PlayerState> EmberPlayerState;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetControllerParams")
	TObjectPtr<UASC_AbilitySystemComponent> EmberAbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetControllerParams")
	TObjectPtr<UAS_BaseAttributeSet> EmberAttributeSet;
	
	//Data Asset of all abilities in game
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "WidgetData")
	TObjectPtr<UDA_AbilityInfo> AbilityInfo;
	
	APS_PlayerState* GetEmberPlayerState();
	UASC_AbilitySystemComponent* GetEmberAbilitySystemComponent();
	UAS_BaseAttributeSet* GetEmberAttributeSet();
};