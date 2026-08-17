// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/DataTable.h"
#include "QuestSystemSettings.generated.h"

UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Quest System Settings"))
class QUESTSUBSYSTEM_API UQuestSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Soft reference ensures the table isn't force-loaded until needed
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data Tables", meta = (AllowedClasses = "/Script/Engine.DataTable"))
	TSoftObjectPtr<UDataTable> QuestDataTable;
};
