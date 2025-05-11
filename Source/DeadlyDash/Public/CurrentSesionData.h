// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FSaveSlotInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CurrentSesionData.generated.h"

/**
 * 
 */
UCLASS()
class DEADLYDASH_API UCurrentSessionData : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static void SetSeed(int32 NewSeed);

	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static int GetSeed();
	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static FString GetSaveSlotName();
	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static void SetSavedSlotsList(const TArray<FSaveSlotInfo> &slots);
	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static TArray<FSaveSlotInfo> GetSavedSlotsList();
	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static int GetLevel();
	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static int IncreaseLevel();
	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static void SetLevel(int NewLevel);
	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static FString GetSessionDescription();
	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static void SetHealth(int health);
	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static int32 GetHealth();
	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static void SetMaxHealth(int health);
	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static int32 GetMaxHealth();
	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static float GetCurrency();
	UFUNCTION(BlueprintCallable, Category = "Session Data")
	static void SetCurrency(float NewCurrency);
private:
	static int32 Seed; // Статическая переменная для хранения seed
	static TArray<FSaveSlotInfo> SavedSlotsList;
	static int Level;
	static int MaxHealth;
	static int Health;
	static int Currency;
};