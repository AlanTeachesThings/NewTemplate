// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Engine/DataTable.h" // Required for FTableRowBase
#include "BaseQuestLogic.h" // Required for BaseQuestLogic
#include "StructUtils/InstancedStruct.h"
#include "QuestTypes.generated.h"

USTRUCT(BlueprintType)
struct FQuestTableRow : public FTableRowBase // Inherit from FTableRowBase
{
	GENERATED_BODY()

	//Quest GetHeadline: Needs to be removed! GetHeadline is now a function of the QuestLogic
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FString GetHeadline;

	//Quest Logic object class to be produced
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	TSubclassOf<UBaseQuestLogic> LogicClass;

	//Quest Logic payload
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FInstancedStruct QuestPayload;

	//Potential Parent Quest
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
	FDataTableRowHandle ParentQuest;
};
