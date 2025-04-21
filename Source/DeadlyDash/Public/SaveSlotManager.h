// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FSaveSlotInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SaveSlotManager.generated.h"

/**
 * 
 */
UCLASS()
class DEADLYDASH_API USaveSlotManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Save System")
	static TArray<FSaveSlotInfo> GetSortedSaveSlots();
	UFUNCTION(BlueprintCallable, Category = "Save System")
	static FString DateTimeToString(const FDateTime& DateTime);

private:
	static bool ParseSlotName(const FString& FileName, int32& OutSeed);
};
