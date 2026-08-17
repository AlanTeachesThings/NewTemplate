// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestSystem.h"
#include "QuestSystemSettings.h"
#include "Misc/CoreDelegates.h"
#include "UObject/Stack.h"
#include "StructUtils/InstancedStruct.h"
#include "BaseQuestLogic.h"

void UQuestSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// Bind to the OnPostLoadMapWithWorld delegate
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UQuestSystem::OnWorldInitialized);
}

void UQuestSystem::Deinitialize()
{
	// Always unbind to prevent dangling references on cleanup
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);

	Super::Deinitialize();
}

void UQuestSystem::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues IVS)
{
	// For each entry in ActiveQuests, recreate the Quest Logic object and re-add it to the map
}

UDataTable* UQuestSystem::GetQuestDataTable()
{
	// 1. Return cached pointer if already loaded
	if (CachedQuestDataTable)
	{
		return CachedQuestDataTable;
	}

	// 2. Read the soft pointer from Project Settings
	const UQuestSystemSettings* Settings = GetDefault<UQuestSystemSettings>();
	if (Settings && !Settings->QuestDataTable.IsNull())
	{
		// Load synchronously (or use FStreamableManager for async loading)
		CachedQuestDataTable = Settings->QuestDataTable.LoadSynchronous();
		return CachedQuestDataTable;
	}

	UE_LOG(LogTemp, Warning, TEXT("QuestSubsystem: QuestDataTable is not assigned in Project Settings!"));
	return nullptr;
}

// GET QUEST DATA
// 1. Get our Quest Data Table (or return null)
// 2. Look up the row defined by QuestID and return it
FQuestTableRow* UQuestSystem::GetQuestDataRow(FName QuestID)
{
	UDataTable* Table = GetQuestDataTable();
	if (!Table) return nullptr;

	static const FString ContextString(TEXT("QuestSubsystemLookup"));
	return Table->FindRow<FQuestTableRow>(QuestID, ContextString);
}

// GET QUEST LOGIC
// 1. Find the pointer-to-pointer that relates to the row (or return null)
// 2. Dereference the pointer-to-pointer into a pointer that we can return safely
UBaseQuestLogic* UQuestSystem::GetQuestLogic(FName QuestName)
{
	// 1. Find returns a pointer-to-pointer: UBaseQuestLogic**
	if (UBaseQuestLogic** FoundLogicPtr = ActiveQuestLogicsByRow.Find(QuestName))
	{
		// 2. Dereference to get the actual UBaseQuestLogic*
		return *FoundLogicPtr;
	}

	// Key was not found in the map
	return nullptr;
}

// ADD QUEST
// 1. Check we can get the requested quest (or return false)
// 2. Check we haven't already added the quest (or return false) [TODO: Fail with more verbose messaging]
// 3. Spawn the QuestLogic object based on the given class
// 4. Call InitialiseLogic with the given payload from the QuestData
// 5. Store a reference to the object in the map using the data table row as a key
// 6. Call our event dispatchers
bool UQuestSystem::AddQuest(FName NewQuestName)
{
	if (ActiveQuestLogicsByRow.Find(NewQuestName))
	{
		return false; // return false if already added TODO: find a way to fail this with more verbose messaging
	}
	else
	{
		FQuestTableRow* NewQuestTableRow = GetQuestDataRow(NewQuestName);  // Get the Table row for NewQuestName
		if (!NewQuestTableRow) return false;							// Return false if we can't find it TODO: find a way to fail this with more verbose messaging
		FName ParentQuestName = NewQuestTableRow->ParentQuest.RowName; 		// Get the Parent Quest row name
		UBaseQuestLogic* ParentQuestLogic = nullptr;
		if (ParentQuestName != "") // If Parent Quest isn't empty, then try and add it, then get the ParentQuestLogic for it
		{
			AddQuest(ParentQuestName);
			ParentQuestLogic = GetQuestLogic(ParentQuestName);
		}
		UBaseQuestLogic* NewQuestLogic = NewObject<UBaseQuestLogic>(this, NewQuestTableRow->LogicClass); // Create the logic object for this quest
		NewQuestLogic->QuestName = NewQuestName; // Make sure Quest knows its own name
		NewQuestLogic->ParentQuestName = ParentQuestName; // Give it its parent quest name ("" if none)
		NewQuestLogic->InitialiseWithPayload(NewQuestTableRow->QuestPayload); // Initialise the quest logic with the payload
		ActiveQuestLogicsByRow.Add(NewQuestName, NewQuestLogic); // Store it in the map
		NewQuestLogic->OnQuestUpdated.AddDynamic(this, &UQuestSystem::QuestUpdatePassthrough); //Map OnQuestUpdateReceived to NewQuestLogic's Update Quest dispatcher
		NewQuestLogic->SetQuestStatus(EQuestStatus::InProgress); // Set status to "In Progress"
		if (ParentQuestLogic)
		{
			ParentQuestLogic->RegisterChildQuest(NewQuestLogic); // If it had a parent logic, then register with it
		}
		OnEventBroadcasted.AddDynamic(NewQuestLogic, &UBaseQuestLogic::ReceiveEvent);
		OnQuestAdded.Broadcast(NewQuestLogic);		//Broadcast the birth of our bouncing baby Quest Logic to the world
		return true;
	}
}

// REMOVE QUEST
// 1. Check we can get the requested quest (or return false)
// 2. Check we haven't already added the quest (or return false) [TODO: Fail with more verbose messaging]
// 3. Call event dispatcher (before destroying object)
// 4. Destroy the object
// 5. Remove the entry from the array
bool UQuestSystem::RemoveQuest(FName QuestNameToRemove)
{
	if (ActiveQuestLogicsByRow.Find(QuestNameToRemove))
	{
		return false; // return false if already added TODO: find a way to fail this with more verbose messaging
	}
	else
	{
		FQuestTableRow* NewQuestTableRow = GetQuestDataRow(QuestNameToRemove);  // Get the Table row for NewQuestName
		if (!NewQuestTableRow) return false;							// Return false if we can't find it TODO: find a way to fail this with more verbose messaging
		UBaseQuestLogic* QuestLogicToDestroy = GetQuestLogic(QuestNameToRemove);
		if (!QuestLogicToDestroy) return false;							// Return false if we can't find it TODO: find a way to fail this with more verbose messaging
		OnQuestRemoved.Broadcast(QuestLogicToDestroy); // Broadcast the death of our quest logic before we kill it
		ActiveQuestLogicsByRow.Remove(QuestNameToRemove); // Remove the reference from the map
		QuestLogicToDestroy->MarkAsGarbage(); //Quite possibly not necessary, but mark it as garbage to be destroyed just in case
		return true;
	}
}

// SET QUEST STATUS
// Just finds the Quest Logic and runs Set Quest Status on it
void UQuestSystem::SetQuestStatus(FName QuestName, EQuestStatus NewStatus)
{
	GetQuestLogic(QuestName)->SetQuestStatus(NewStatus);
}

// GET QUEST HEADLINE
// 1. Find the Data Table Row that relates to the QuestName
// 2. Find the Quest Logic that relates to the row
// 3. Return the GetHeadline string function from that logic
FString UQuestSystem::GetQuestHeadline(FName QuestName)
{
	FQuestTableRow* HeadlineRow = GetQuestDataRow(QuestName);
	if (HeadlineRow)
	{
		UBaseQuestLogic* HeadlineQuestLogic = GetQuestLogic(QuestName);
		if (HeadlineQuestLogic)
		{
			return HeadlineQuestLogic->GetHeadline();
		}
		else
		{
			return "No logic found";
		}
	}
	else
	{
		return "No logic found";
	}
}

// Basic Data functions
// Get an array of active Quest Names
TArray<FName> UQuestSystem::GetActiveQuests()
{
	TArray<FName> QuestNamesToReturn;
	ActiveQuestLogicsByRow.GenerateKeyArray(QuestNamesToReturn);
	return QuestNamesToReturn;
}

// Get the data table row info for a Quest Name
FQuestTableRow UQuestSystem::GetQuestData(FName QuestName)
{
	return *GetQuestDataRow(QuestName);
}

// Simple function to call the system's OnQuestUpdate when an individual quest updates
void UQuestSystem::QuestUpdatePassthrough(UBaseQuestLogic* UpdatedQuestLogic)
{
	OnQuestUpdateReceived.Broadcast(UpdatedQuestLogic);
}

// EVENT SYSTEM
void UQuestSystem::BroadcastEvent(const int32& AnyStruct)
{
	// Never called at runtime because CustomThunk routes directly to execProcessAnyStruct
	checkNoEntry();
}

DEFINE_FUNCTION(UQuestSystem::execBroadcastEvent)
{
	// Step 1: Step through the stack to evaluate the wildcard parameter
	Stack.StepCompiledIn(nullptr, nullptr);

	FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	void* StructDataPtr = Stack.MostRecentPropertyAddress;

	P_FINISH; // Mark parameter reading complete before executing custom logic

	if (!StructProperty || !StructDataPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("BroadCastEvent: Invalid struct property or null data pointer passed."));
		return;
	}

	// Step 2: Retrieve the reflection metadata (UScriptStruct)
	UScriptStruct* StructType = StructProperty->Struct;

	if (!StructType)
	{
		return;
	}

	// Step 3: Convert to FInstancedStruct
	// FInstancedStruct::Make copies the raw struct memory using its UScriptStruct reflection info
	FInstancedStruct OutInstancedStruct;
	OutInstancedStruct.InitializeAs(StructType, static_cast<const uint8*>(StructDataPtr));

	// Example Usage: Verify and log
	UE_LOG(LogTemp, Log, TEXT("Successfully converted '%s' to FInstancedStruct!"), *StructType->GetName());

	//Broadcast Instanced Struct as Event
	P_THIS->OnEventBroadcasted.Broadcast(OutInstancedStruct);
}


