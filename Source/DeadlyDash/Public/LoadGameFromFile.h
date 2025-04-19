// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LoadGameFromFile.generated.h"

/**
 * 
 */
UCLASS()
class DEADLYDASH_API ULoadGameFromFile : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "File Operations", meta = (DisplayName = "Load JSON And Start Level"))
	static bool LoadJSONAndStartLevel();
private:
	static FVector LoadedPlayerPosition;
	static void OnLevelLoaded(UWorld* LoadedWorld);
};
