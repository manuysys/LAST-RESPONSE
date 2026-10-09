#include "Player/LRInventoryComponent.h"

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
}

bool ULRInventoryComponent::RemoveTool(FName ToolId)
{
	if (ToolIds.Remove(ToolId) > 0)
	{
		OnToolRemoved.Broadcast(ToolId);
		return true;
	}

	return false;
}
