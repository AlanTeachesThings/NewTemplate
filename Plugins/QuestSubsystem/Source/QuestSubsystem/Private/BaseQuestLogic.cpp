// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseQuestLogic.h"
#include "StructUtils/InstancedStruct.h"


FString UBaseQuestLogic::GetHeadline_Implementation()
{
	return "Base Quest Logic GetHeadline";
}

void UBaseQuestLogic::InitialiseWithPayload_Implementation(FInstancedStruct Payload)
{

}

void UBaseQuestLogic::ReceiveEvent_Implementation(FInstancedStruct EventPayload)
{

}

void UBaseQuestLogic::UpdateQuest_Implementation() // Base UpdateQuest function just broadcasts that a Quest has updated
{
	OnQuestUpdated.Broadcast(this);
}

// SET QUEST STATUS
// Run through various starting & requested status pairs and broadcast changes appropriately
void UBaseQuestLogic::SetQuestStatus(EQuestStatus NewStatus)
{
	if (NewStatus == EQuestStatus::Failed && QuestStatus != EQuestStatus::Failed) // Asking to Fail Quest & Quest is not already failed
	{
		if (QuestStatus == EQuestStatus::Completed) // If failing from Completed then we're also Un-Completing
		{
			QuestStatus = NewStatus;
			OnQuestUnCompleted.Broadcast(this);
			OnQuestUpdated.Broadcast(this);
		}
		else
		{
			QuestStatus = NewStatus;
			OnQuestFailed.Broadcast(this);
			OnQuestUpdated.Broadcast(this);
		}
	}
	
	else if (NewStatus == EQuestStatus::Completed && QuestStatus != EQuestStatus::Completed) // Asking to Complete Quest & Quest is not already completed
	{
		QuestStatus = NewStatus;
		OnQuestCompleted.Broadcast(this);
		OnQuestUpdated.Broadcast(this);
	}
	else
	{
		if (NewStatus != QuestStatus) // Final check that the status is actually changing
		{
			if (QuestStatus == EQuestStatus::Completed) // If changing from Completed then we're Un-Completing
			{
				QuestStatus = NewStatus;
				OnQuestUnCompleted.Broadcast(this);
				OnQuestUpdated.Broadcast(this);
			}
			else
			{
				QuestStatus = NewStatus;
				OnQuestUpdated.Broadcast(this);
			}
		}
	}
}

// REGISTER CHILD QUESTS
	// So that the parent quest logic can check them
void UBaseQuestLogic::RegisterChildQuest(UBaseQuestLogic* ChildQuest)
{
	if (!ChildQuest)
	{
		return;
	}

	ChildQuests.AddUnique(ChildQuest);

	// Prevent duplicate delegate bindings
	if (!ChildQuest->OnQuestUpdated.IsAlreadyBound(this, &UBaseQuestLogic::CallUpdateQuest))
	{
		ChildQuest->OnQuestUpdated.AddDynamic(this, &UBaseQuestLogic::CallUpdateQuest); // For some reason, this isn't working?
	}
}

void UBaseQuestLogic::CallUpdateQuest(UBaseQuestLogic* BaseQuestLogic)
{
	UpdateQuest();
}