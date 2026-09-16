// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#pragma once
#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "DialogueFactory.generated.h"

UCLASS()
class CUSTOMEDITOR_API UDialogueFactory : public UFactory
{
	GENERATED_BODY()
	
public:
	UDialogueFactory();

	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	
	virtual bool FactoryCanImport(const FString& Filename) override;
};