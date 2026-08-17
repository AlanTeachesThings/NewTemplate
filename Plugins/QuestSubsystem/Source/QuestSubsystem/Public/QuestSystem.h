// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuestTypes.h"
#include "QuestSystem.generated.h"

//Declare Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestAddedSignature, UBaseQuestLogic*, AddedQuestLogic);	// To be called when adding a quest
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestRemovedSignature, UBaseQuestLogic*, RemovedQuestLogic);	// To be called when removing a quest
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestUpdateReceivedSignature, UBaseQuestLogic*, UpdatedQuestLogic);	// To be called when any quest object calls their OnQuestUpdated

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEventBroadcastedSignature, FInstancedStruct, EventPayload);	// To be called as events throughout the game


/**
 * 
 */
UCLASS(BlueprintType)
class QUESTSUBSYSTEM_API UQuestSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// Called when the subsystem is created
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// Called when the subsystem is destroyed
	virtual void Deinitialize() override;

	//Expose Delegates to BPs as Event Dispatchers
	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnQuestAddedSignature OnQuestAdded;
	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnQuestRemovedSignature OnQuestRemoved;
	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnQuestUpdateReceivedSignature OnQuestUpdateReceived;

	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnEventBroadcastedSignature OnEventBroadcasted;

	// Function to call OnQuestUpdated when a Quest Logic reports being updated
	UFUNCTION()
	void QuestUpdatePassthrough(UBaseQuestLogic* UpdatedQuestLogic);

	// Quest Storage
	UPROPERTY()
	TMap<FName, UBaseQuestLogic*> ActiveQuestLogicsByRow; // Map of Quest Logic objects by Table Rows

	// Helper to load or fetch the active Data Table
	UDataTable* GetQuestDataTable();

	//Row Fetching Function (fetch by QuestName)
	FQuestTableRow* GetQuestDataRow(FName QuestName);

	//Quest Logic Fetching Function
	UFUNCTION(BlueprintPure, Category = "Quest System")
	UBaseQuestLogic* GetQuestLogic(FName QuestName);

	//Quest System Functions
	UFUNCTION(BlueprintCallable, Category = "Quest System")
	bool AddQuest(FName NewQuestName); // Add a Quest to the stack (checks if valid/already added)
	UFUNCTION(BlueprintCallable, Category = "Quest System")
	bool RemoveQuest(FName NewQuestName); // Remove a Quest from the stack
	UFUNCTION(BlueprintCallable, Category = "Quest System")
	void SetQuestStatus(FName QuestName, EQuestStatus NewStatus);

	//Quest Data Retrieval Functions
	UFUNCTION(BlueprintPure, Category = "Quest System")
	TArray<FName> GetActiveQuests(); // Get all active quest names (keys from ActiveQuestLogicsByRow)
	UFUNCTION(BlueprintPure, Category = "Quest System")
	FString GetQuestHeadline(FName QuestName); // Get the headline text for a given quest name
	UFUNCTION(BlueprintPure, Category = "Quest System")
	FQuestTableRow GetQuestData(FName QuestName); // Get the Quest Data for the quest

	//Event System - a single event taking a wildcard struct
	UFUNCTION(BlueprintCallable, CustomThunk, meta = (CustomStructureParam = "AnyStruct", Mode = "Read"), Category="Event System")
	void BroadcastEvent(const int32& AnyStruct);
	DECLARE_FUNCTION(execBroadcastEvent);

private:
	// Delegate handler function for when a world is loaded
	void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS);

	// Cache hard pointer for Data Table once loaded so we don't keep loading it synchronously
	UPROPERTY()
	TObjectPtr<UDataTable> CachedQuestDataTable;

};
