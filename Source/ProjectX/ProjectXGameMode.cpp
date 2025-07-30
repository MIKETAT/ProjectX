// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectXGameMode.h"
#include "UObject/ConstructorHelpers.h"

AProjectXGameMode::AProjectXGameMode()
{
	/*// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}*/
}

void AProjectXGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (!HasAuthority() || !NewPlayer)
	{
		return;
	}
	if (NewPlayer->GetPawn() == nullptr)
	{
		TSubclassOf<APawn> PawnClass = DefaultPawnClass;
		if (PawnClass)
		{
			FTransform SpawnTransform;
			// 使用 PlayerStart 或指定位置，你可根据项目自行调整
			AActor* PlayerStart = FindPlayerStart(NewPlayer);
			if (PlayerStart)
			{
				SpawnTransform = PlayerStart->GetActorTransform();
			}
			// Spawn Pawn
			FActorSpawnParameters Params;
			Params.Instigator = nullptr;
			Params.Owner = NewPlayer;
			APawn* NewPawn = GetWorld()->SpawnActor<APawn>(PawnClass, SpawnTransform, Params);
			if (NewPawn)
			{
				NewPlayer->Possess(NewPawn);
			}
		}
	}
}
