#include "Items/ItemBase.h"

#include "InventoryStatics.h"
#include "ItemDefinition.h"
#include "XPlayerController.h"
#include "Character/CharacterBase.h"
#include "Component/InventoryComponent.h"
#include "Components/SphereComponent.h"

AItemBase::AItemBase()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Item"));
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(SphereCollision);
	ItemMesh->SetupAttachment(SphereCollision);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereBeginOverlap);
	SphereCollision->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnSphereEndOverlap);
	ItemMesh->SetIsReplicated(true);
	SphereCollision->SetIsReplicated(true);
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();
}


void AItemBase::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Error, TEXT("OnSphereBeginOverlap called on : %s"), HasAuthority() ? TEXT("Authority") : TEXT("Client"));
	if (ACharacterBase* CharacterBase = Cast<ACharacterBase>(OtherActor))
	{
		CharacterBase->SetOverlapItem(this);
		// 先取消可见和碰撞
		if (AXPlayerController* PlayerController = Cast<AXPlayerController>(CharacterBase->GetController()))
		{
			if (UInventoryComponent* InventoryComponent = UInventoryStatics::GetInventoryComponent(PlayerController))
			{
				
			}
		}
	}
}


void AItemBase::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ensureMsgf(ItemMesh, TEXT("ItemMesh is null!"));
	ensureMsgf(SphereCollision, TEXT("SphereCollision is null!"));
	if (ACharacterBase* CharacterBase = Cast<ACharacterBase>(OtherActor))
	{
		CharacterBase->SetOverlapItem(nullptr);
	}
	
}

void AItemBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
