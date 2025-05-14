// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MainMenuSettingsController.generated.h"

/**
 * 
 */
UCLASS()
class DEADLYDASH_API UMainMenuSettingsController : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Сохранение/загрузка
	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	static void SaveGameSettings();

	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	static void LoadGameSettings();

	// Геттеры

	UFUNCTION(BlueprintPure, Category = "Game Settings")
	static FString GetCurrentResolution();

	UFUNCTION(BlueprintPure, Category = "Game Settings")
	static TArray<FString> GetSupportedResolutions();

	UFUNCTION(BlueprintPure, Category = "Game Settings")
	static TArray<FString> GetSupportedTextureQualities();
	
	UFUNCTION(BlueprintPure, Category = "Game Settings")
	static FString GetTextureQuality();

	UFUNCTION(BlueprintPure, Category = "Game Settings")
	static float GetMouseSensitivity();

	UFUNCTION(BlueprintPure, Category = "Game Settings")
	static float GetMasterVolume();

	UFUNCTION(BlueprintPure, Category = "Game Settings")
	static float GetMusicVolume();

	// Сеттеры
	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	static void SetScreenResolution(FString NewResolution);

	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	static void SetTextureQuality(FString NewQuality);

	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	static void SetMouseSensitivity(float NewSensitivity);

	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	static void SetMasterVolume(float NewVolume);

	UFUNCTION(BlueprintCallable, Category = "Game Settings")
	static void SetMusicVolume(float NewVolume);

	static FString GetConfigPath();
private:
	static FString Resolution;
	static FString TextureQuality;
	static float MouseSensitivity;
	static float MasterVolume;
	static float MusicVolume;
};