// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuSettingsController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "GameFramework/GameUserSettings.h"

void UMainMenuSettingsController::SetScreenResolution(FString NewResolution)
{
    FString WidthStr, HeightStr;
    if (NewResolution.Split(TEXT("x"), &WidthStr, &HeightStr))
    {
        const int32 Width = FCString::Atoi(*WidthStr);
        const int32 Height = FCString::Atoi(*HeightStr);
        
        if (Width > 0 && Height > 0)
        {
            if (UGameUserSettings* GameSettings = UGameUserSettings::GetGameUserSettings())
            {
                GameSettings->SetScreenResolution(FIntPoint(Width, Height));
                GameSettings->ApplySettings(false);
            }
        }
    }
    Resolution = NewResolution;
}

FString UMainMenuSettingsController::GetCurrentResolution()
{
    return Resolution;
}

TArray<FString> UMainMenuSettingsController::GetSupportedResolutions()
{
    TArray<FIntPoint> Resolutions;
    TArray<FString> Result;

    UKismetSystemLibrary::GetSupportedFullscreenResolutions(Resolutions);
    
    for (const FIntPoint& Res : Resolutions)
    {
        Result.Add(FString::Printf(TEXT("%dx%d"), Res.X, Res.Y));
    }

    return Result;
}

// Инициализация статических переменных
FString UMainMenuSettingsController::Resolution = "1920x1080";
FString UMainMenuSettingsController::TextureQuality = "High";
float UMainMenuSettingsController::MouseSensitivity = 1.2f;
float UMainMenuSettingsController::MasterVolume = 80.0f;
float UMainMenuSettingsController::MusicVolume = 60.0f;

void UMainMenuSettingsController::SaveGameSettings()
{
    FConfigFile ConfigFile;
    ConfigFile.Read(GetConfigPath());
    ConfigFile.SetString(TEXT("Graphics"), TEXT("Resolution"), *Resolution);
    ConfigFile.SetString(TEXT("Graphics"), TEXT("TextureQuality"), *TextureQuality);
    ConfigFile.SetFloat(TEXT("Controls"), TEXT("MouseSensitivity"), MouseSensitivity);
    ConfigFile.SetFloat(TEXT("Audio"), TEXT("MasterVolume"), MasterVolume);
    ConfigFile.SetFloat(TEXT("Audio"), TEXT("MusicVolume"), MusicVolume);
    
    ConfigFile.Write(GetConfigPath());
}

void UMainMenuSettingsController::LoadGameSettings()
{
    FConfigFile ConfigFile;
    ConfigFile.Read(GetConfigPath());
    ConfigFile.GetString(TEXT("Graphics"), TEXT("Resolution"), Resolution);
    ConfigFile.GetString(TEXT("Graphics"), TEXT("TextureQuality"), TextureQuality);
    ConfigFile.GetFloat(TEXT("Controls"), TEXT("MouseSensitivity"), MouseSensitivity);
    ConfigFile.GetFloat(TEXT("Audio"), TEXT("MasterVolume"), MasterVolume);
    ConfigFile.GetFloat(TEXT("Audio"), TEXT("MusicVolume"), MusicVolume);
}

// Геттеры
FString UMainMenuSettingsController::GetTextureQuality() { return TextureQuality; }

TArray<FString> UMainMenuSettingsController::GetSupportedTextureQualities() {
    return TArray<FString> { "High", "Normal", "Low" };
}
float UMainMenuSettingsController::GetMouseSensitivity() { return MouseSensitivity; }
float UMainMenuSettingsController::GetMasterVolume() { return MasterVolume; }
float UMainMenuSettingsController::GetMusicVolume() { return MusicVolume; }

// Сеттеры
void UMainMenuSettingsController::SetTextureQuality(FString NewQuality)
{
    TextureQuality = NewQuality;
}
void UMainMenuSettingsController::SetMouseSensitivity(float NewSensitivity) { MouseSensitivity = FMath::Clamp(NewSensitivity, 0.1f, 5.0f); }
void UMainMenuSettingsController::SetMasterVolume(float NewVolume) { MasterVolume = FMath::Clamp(NewVolume, 0.0f, 100.0f); }
void UMainMenuSettingsController::SetMusicVolume(float NewVolume) { MusicVolume = FMath::Clamp(NewVolume, 0.0f, 100.0f); }
FString UMainMenuSettingsController::GetConfigPath() {
    return FPaths::ProjectSavedDir() + TEXT("Config/GameSettings.ini");
}


