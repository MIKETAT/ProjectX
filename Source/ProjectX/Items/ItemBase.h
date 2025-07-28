// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBase.generated.h"

class USphereComponent;
class UItemDefinition;

UCLASS()
class PROJECTX_API AItemBase : public AActor
{
	GENERATED_BODY()
public:	
	AItemBase();
	TSubclassOf<UItemDefinition> GetItemDefinition() const { return ItemDefinitionClass; }
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<UItemDefinition> ItemDefinitionClass;
	
	UPROPERTY(EditAnywhere, Category = "Inventory")
	TObjectPtr<UStaticMeshComponent> ItemMesh;

	UPROPERTY(EditAnywhere, Category = "Inventory")
	TObjectPtr<USphereComponent> SphereCollision;	// 物品拾取碰撞体积
	
#if WITH_EDITOR
	// 编辑器内立即生效
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
};
