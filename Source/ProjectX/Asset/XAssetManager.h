// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "ProjectX.h"
#include "XAssetManager.generated.h"

UCLASS(BlueprintType, Blueprintable)
class PROJECTX_API UXAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:
	/** 获取AssetManager全局单例 */
	static UXAssetManager& Get() {
		check(GEngine);
		if (UXAssetManager* Singleton = Cast<UXAssetManager>(GEngine->AssetManager)) {
			return *Singleton;
		}
		UE_LOG(ProjectX, Error, TEXT("UXAssetManager is not set as the AssetManager in the engine."));
		return *NewObject<UXAssetManager>();
	}

	template <typename AssetClass>
	AssetClass* GetAssetSync(const TSoftObjectPtr<AssetClass>& AssetPtr)
	{
		return AssetPtr.LoadSynchronous();
	}
};
