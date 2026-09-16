// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "DialogueFactory.h"
#include "Narrative/C_DialogueInteraction.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

UDialogueFactory::UDialogueFactory()
{
	SupportedClass = UC_DialogueInteraction::StaticClass();
	bCreateNew = false;
	bEditorImport = true;
	Formats.Add(TEXT("json;Dialogue Interaction JSON"));
}

bool UDialogueFactory::FactoryCanImport(const FString& Filename)
{
	return Filename.EndsWith(TEXT(".json"));
}

// Parse JSON file
UObject* UDialogueFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
    bOutOperationCanceled = false;

    FString FileContents;
    if (!FFileHelper::LoadFileToString(FileContents, *Filename))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load JSON file: %s"), *Filename);
        return nullptr;
    }

    TSharedPtr<FJsonObject> Root;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContents);

    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid JSON: %s"), *Filename);
        return nullptr;
    }

    // Create the asset
    UC_DialogueInteraction* Asset = NewObject<UC_DialogueInteraction>(InParent, InClass, InName, Flags);

    // Interaction ID
    FString InteractionId;
    if (Root->TryGetStringField(TEXT("interaction_id"), InteractionId))
        Asset->InteractionId = FName(*InteractionId);

    // Lines array
    const TArray<TSharedPtr<FJsonValue>>* LinesJson;
    if (Root->TryGetArrayField(TEXT("lines"), LinesJson))
    {
        for (const TSharedPtr<FJsonValue>& Value : *LinesJson)
        {
            const TSharedPtr<FJsonObject> LineObj = Value->AsObject();
            if (!LineObj.IsValid()) continue;

            FDialogueLine Line;

            // Speaker
            FString Speaker;
            if (LineObj->TryGetStringField(TEXT("speaker"), Speaker))
                Line.Speaker = FText::FromString(Speaker);

            // Dialogue Text
            FString Text;
            if (LineObj->TryGetStringField(TEXT("text"), Text))
                Line.Text = FText::FromString(Text);

            // Dialogue Duration
            LineObj->TryGetNumberField(TEXT("duration"), Line.Duration);

            Asset->Lines.Add(Line);
        }
    }

    return Asset;
}