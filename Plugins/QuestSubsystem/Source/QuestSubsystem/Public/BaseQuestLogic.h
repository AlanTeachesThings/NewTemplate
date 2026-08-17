// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/NoExportTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "BaseQuestLogic.generated.h"

/**
 * 
 */

 //Declare Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestUpdatedSignature, UBaseQuestLogic*, UpdatedQuest);	// To be called when adding a quest
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompletedSignature, UBaseQuestLogic*, CompletedQuest);	// To be called when a quest is completed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestUnCompletedSignature, UBaseQuestLogic*, UnCompletedQuest);	// To be called when a quest is un-completed
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailedSignature, UBaseQuestLogic*, UpdatedQuest);	// To be called when a quest is failed

//Quest Status enum
UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
	NotStarted	UMETA(DisplayName = "Not Started"),
	InProgress	UMETA(DisplayName = "In Progress"),
	Completed	UMETA(DisplayName = "Completed"),
	Failed		UMETA(DisplayName = "Failed")
};

UCLASS(Blueprintable, BlueprintType)
class QUESTSUBSYSTEM_API UBaseQuestLogic : public UObject
{
	GENERATED_BODY()

public:

	//Expose Delegates to BPs as Event Dispatchers
	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnQuestUpdatedSignature OnQuestUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnQuestCompletedSignature OnQuestCompleted;
	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnQuestUnCompletedSignature OnQuestUnCompleted;
	UPROPERTY(BlueprintAssignable, Category = "Quest Events")
	FOnQuestFailedSignature OnQuestFailed;

	//Child Quest list - to allow nested quests
	UPROPERTY(BlueprintReadOnly, Category = "Quest Data")
	TArray<UBaseQuestLogic*> ChildQuests;

	//InitialiseWithPayload - a function that's called right after the object is created, taking in an Instanced Struct to allow for variables to be set up in each sub-class
	UFUNCTION(BlueprintNativeEvent, Category = "Quest Setup")
	void InitialiseWithPayload(FInstancedStruct Payload);
	virtual void InitialiseWithPayload_Implementation(FInstancedStruct Payload);

	//Update Quest - a function that should be called by the BP whenever the quest updates, which will then fire the necessary event dispatcher to update other things via the Quest Subsystem
	//InitialiseWithPayload - a function that's called right after the object is created, taking in an Instanced Struct to allow for variables to be set up in each sub-class
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Quest Setup")
	void UpdateQuest();
	virtual void UpdateQuest_Implementation();

	UFUNCTION()
	void CallUpdateQuest(UBaseQuestLogic* BaseQuestLogic); //Helper function for when binding children and parents

	//HEADLINE - a Blueprint-overrideable function to return the text that should go in the UI
	//GetHeadline declaration for C++
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, BlueprintCallable, Category = "Quest Data")
	FString GetHeadline();
	virtual FString GetHeadline_Implementation();

	//QuestName - The Quest Logic should know its name so you can refer back to the Quest Table if necessary
	UPROPERTY(BlueprintReadOnly, Category = "Quest Data")
	FName QuestName;

	//ParentQuestName - Handy to know if this is part of a parent quest or not 
	UPROPERTY(BlueprintReadOnly, Category = "Quest Data")
	FName ParentQuestName;

	//Quest Status Property
	UPROPERTY(BlueprintReadOnly, Category = "Quest Data")
	EQuestStatus QuestStatus = EQuestStatus::NotStarted;
	
	//Quest Status Function for updating (allows for calling Delegates)
	UFUNCTION(BlueprintCallable, Category = "Quest Events")
	void SetQuestStatus(EQuestStatus NewStatus);

	//Register Child Quests in case of Parent Quest
	UFUNCTION(BlueprintCallable, Category = "Quest Events")
	void RegisterChildQuest(UBaseQuestLogic* ChildQuest);

	//Function for receiving events from Quest system
	UFUNCTION(BlueprintNativeEvent, Category = "Events")
	void ReceiveEvent(FInstancedStruct EventPayload);
};
