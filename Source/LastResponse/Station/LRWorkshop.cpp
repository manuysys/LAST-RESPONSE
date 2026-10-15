#include "Station/LRWorkshop.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Meta/LRProgressionSubsystem.h"
#include "Player/LRPlayerCharacter.h"
#include "Player/LRInventoryComponent.h"
#include "Prototype/LRPrototypeVisuals.h"
#include "Vehicle/LRRescueVehicle.h"

ALRWorkshop::ALRWorkshop()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	WorkshopMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WorkshopMesh"));
	SetRootComponent(WorkshopMesh);
	WorkshopMesh->SetRelativeScale3D(FVector(3.2f, 2.2f, 1.5f));
	if (UStaticMesh* Cube = FLRProto::CubeMesh())
	{
		WorkshopMesh->SetStaticMesh(Cube);
	}
}

void ALRWorkshop::BeginPlay()
{
	Super::BeginPlay();

	FLRProto::ApplyColor(WorkshopMesh, FLinearColor(0.45f, 0.4f, 0.25f));
}

void ALRWorkshop::ServerRepair()
{
	if (!HasAuthority())
	{
		return;
	}

	ULRProgressionSubsystem* Progression = GetGameInstance() ? GetGameInstance()->GetSubsystem<ULRProgressionSubsystem>() : nullptr;
	if (!Progression || !Progression->TrySpendBudget(RepairCost))
	{
		OnRepairDenied.Broadcast();
		return;
	}

	Progression->SetStoredVehicleCondition(100.f);

	for (TActorIterator<ALRRescueVehicle> It(GetWorld()); It; ++It)
	{
		It->RepairToFull();
	}

	OnRepairCompleted.Broadcast();
}

void ALRWorkshop::ServerBuy(ALRPlayerCharacter* Buyer)
{
	if (!HasAuthority() || !Buyer)
	{
		return;
	}

	ULRProgressionSubsystem* Progression = GetGameInstance() ? GetGameInstance()->GetSubsystem<ULRProgressionSubsystem>() : nullptr;
	ULRInventoryComponent* Inventory = Buyer->GetInventoryComponent();

	if (!Progression || !Inventory || !Progression->TrySpendBudget(RestockCost))
	{
		OnPurchaseDenied.Broadcast();
		return;
	}

	for (const FName& ToolId : RestockTools)
	{
		Inventory->AddTool(ToolId);
	}

	OnPurchaseCompleted.Broadcast();
}
