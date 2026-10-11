#include "Player/LRInventoryComponent.h"
#include "Net/UnrealNetwork.h"

ULRInventoryComponent::ULRInventoryComponent()
{
	SetIsReplicatedByDefault(true);
}

void ULRInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULRInventoryComponent, ToolIds);
}

bool ULRInventoryComponent::HasTool(FName ToolId) const
{
	return ToolIds.Contains(ToolId);
}

void ULRInventoryComponent::AddTool(FName ToolId)
{
	if (ToolId == NAME_None || ToolIds.Contains(ToolId))
	{
		return;
	}

	ToolIds.Add(ToolId);
	OnToolAdded.Broadcast(ToolId);
	OnInventoryUpdated.Broadcast();
}

bool ULRInventoryComponent::RemoveTool(FName ToolId)
{
	if (ToolIds.Remove(ToolId) > 0)
	{
		OnToolRemoved.Broadcast(ToolId);
		OnInventoryUpdated.Broadcast();
		return true;
	}

	return false;
}

void ULRInventoryComponent::SetTools(const TArray<FName>& NewTools)
{
	ToolIds = NewTools;
	OnInventoryUpdated.Broadcast();
}

void ULRInventoryComponent::OnRep_ToolIds()
{
	OnInventoryUpdated.Broadcast();
}
