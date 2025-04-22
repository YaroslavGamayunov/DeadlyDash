// Fill out your copyright notice in the Description page of Project Settings.


#include "GameLoader.h"

#include "CurrentSesionData.h"
#include "DesktopPlatformModule.h"
#include "IDesktopPlatform.h"
#include "Kismet/GameplayStatics.h"

FVector UGameLoader::LoadedPlayerPosition = FVector::ZeroVector;

bool UGameLoader::StartLevel(int32 seed, int32 level, FVector position)
{
    const int32 MaxLevels = 5; 
    const int32 ComputedLevel = (level + (seed % 1000)) % MaxLevels + 1;
    const FString LevelName = FString::Printf(TEXT("Procedural_lvl_%d.umap"), ComputedLevel);
    
    const FString LevelPath = FString::Printf(
        TEXT("/Game/Main_Game/Levels/Gameplay_levels_CodeZone/Procedural/Procedural_lvl_%d"),
        ComputedLevel
    );

    // Проверка существования уровня
    if (!FPackageName::DoesPackageExist(LevelPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Level %s does not exist!"), *LevelPath);
        return false;
    }


    // Сохраняем позицию в статической переменной
    LoadedPlayerPosition = position;

    // Загрузка уровня
    UWorld* World = GWorld;
    if (!World) return false;

    // Подписываемся на событие завершения загрузки уровня
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddStatic(&OnLevelLoaded);

    UGameplayStatics::OpenLevel(World, FName(*LevelPath));
    return true;
}


bool UGameLoader::LoadJSONAndStartLevel(FString Path)
{
    // Получаем десктопную платформу
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (!DesktopPlatform) return false;

    // // Open file dialog
    // TArray<FString> SelectedFiles;
    // const bool bFileSelected = DesktopPlatform->OpenFileDialog(
    //     FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
    //     TEXT("Select Save File"),
    //     FPaths::ProjectSavedDir(),
    //     TEXT(""),
    //     TEXT("JSON Files (*.json)|*.json"),
    //     EFileDialogFlags::None,
    //     SelectedFiles
    // );

    // if (!bFileSelected || SelectedFiles.IsEmpty()) return false;

    // Читаем содержимое файла
    FString FileContent;
    if (!FFileHelper::LoadFileToString(FileContent, *Path))
    {
        return false;
    }

    // Парсим JSON
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);
    
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        return false;
    }
    
    const TSharedPtr<FJsonObject>* WorldObject = nullptr;
    if (!JsonObject->TryGetObjectField(TEXT("world"), WorldObject))
    {
        return false;
    }

    int32 LevelNumber = 0;
    int32 Seed = 0;
    if (!(*WorldObject)->TryGetNumberField(TEXT("level_number"), LevelNumber) ||
        !(*WorldObject)->TryGetNumberField(TEXT("seed"), Seed))
    {
        return false;
    }
    UCurrentSessionData::SetSeed(Seed); 

    // Извлекаем позицию
    const TSharedPtr<FJsonObject>* PlayerObject = nullptr;
    if (!JsonObject->TryGetObjectField(TEXT("player"), PlayerObject))
    {
        return false;
    }

    const TSharedPtr<FJsonObject>* PositionObject = nullptr;
    if (!(*PlayerObject)->TryGetObjectField(TEXT("position"), PositionObject))
    {
        return false;
    }

    double X = 0.0, Y = 0.0, Z = 0.0;
    if (!(*PositionObject)->TryGetNumberField(TEXT("x"), X) ||
        !(*PositionObject)->TryGetNumberField(TEXT("y"), Y) ||
        !(*PositionObject)->TryGetNumberField(TEXT("z"), Z))
    {
        return false;
    }

    return StartLevel(LevelNumber, Seed, FVector(X, Y, Z));
}

void UGameLoader::OnLevelLoaded(UWorld* LoadedWorld)
{
    // Отписываемся от события
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(&LoadedWorld);

    // Устанавливаем позицию игроку
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(LoadedWorld, 0))
    {
        if (!LoadedPlayerPosition.IsZero()) {
            PlayerPawn->SetActorLocation(LoadedPlayerPosition);
            UE_LOG(LogTemp, Log, TEXT("Player position set to: %s"), *LoadedPlayerPosition.ToString());
        }
    }
    
    // Очищаем позицию
    LoadedPlayerPosition = FVector::ZeroVector;
}
