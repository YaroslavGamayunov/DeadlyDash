#include "GameLoader.h"

#include "CurrentSesionData.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
//#include "YourPlayerCharacter.h" // Замените на ваш класс персонажа

FVector UGameLoader::LoadedPlayerPosition = FVector::ZeroVector;
FDelegateHandle UGameLoader::LevelLoadedHandle;

bool UGameLoader::StartLevel(int32 level, int32 seed, FVector position)
{
    int32 Cycle = (level - 1) % 3;
    int32 ProceduralNumber = (level - 1) / 3 + 1;
    FString LevelPath;

    constexpr int32 MaxLevels = 9;
    const int32 ComputedLevel = (level + (seed % 1000)) % MaxLevels + 1;

    switch (Cycle)
    {
    case 0:
        LevelPath = FString::Printf(
       TEXT("/Game/Main_Game/Levels/Gameplay_levels_CodeZone/Procedural/Procedural_lvl_%d"),
       ComputedLevel);
        break;
    case 1:
        LevelPath = TEXT("/Game/Main_Game/Levels/Narrative_levels/OrgZone/OrgZone_Main/L_OrgZone_Main.umap");
        break;
    case 2:
        LevelPath = TEXT("/Game/Main_Game/Levels/Hub_levels/L_InstructorZone.umap");
        break;
    default:
        UE_LOG(LogTemp, Error, TEXT("Invalid level cycle"));
        return false;
    }

    if (!FPackageName::DoesPackageExist(LevelPath))
    {
        UE_LOG(LogTemp, Error, TEXT("Level %s not found"), *LevelPath);
        return false;
    }

    LoadedPlayerPosition = position;
    UCurrentSessionData::SetSeed(seed);

    UWorld* World = GWorld;
    if (!World) return false;

    LevelLoadedHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddStatic(&UGameLoader::OnLevelLoaded);
    UGameplayStatics::OpenLevel(World, FName(*LevelPath));

    return true;
}

bool UGameLoader::LoadJSONAndStartLevel(FString Path)
{
    FString FileContent;
    //auto AbsoulutePath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*Path);
    FFileHelper::LoadFileToString(FileContent, *Path);

    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);
    if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid JSON format"));
        return false;
    }

    // Извлечение данных уровня
    const TSharedPtr<FJsonObject>* WorldObject;
    if (!JsonObject->TryGetObjectField(TEXT("world"), WorldObject))
    {
        UE_LOG(LogTemp, Error, TEXT("Missing 'world' field"));
        return false;
    }

    int32 LevelNumber, Seed;
    if (!(*WorldObject)->TryGetNumberField(TEXT("level_number"), LevelNumber) ||
        !(*WorldObject)->TryGetNumberField(TEXT("seed"), Seed))
    {
        UE_LOG(LogTemp, Error, TEXT("Missing level or seed"));
        return false;
    }

    // Извлечение данных игрока
    const TSharedPtr<FJsonObject>* PlayerObject;
    if (!JsonObject->TryGetObjectField(TEXT("player"), PlayerObject))
    {
        UE_LOG(LogTemp, Error, TEXT("Missing 'player' field"));
        return false;
    }

    // Позиция
    const TSharedPtr<FJsonObject>* PositionObject;
    if (!(*PlayerObject)->TryGetObjectField(TEXT("position"), PositionObject))
    {
        UE_LOG(LogTemp, Error, TEXT("Missing position"));
        return false;
    }

    double X, Y, Z;
    if (!(*PositionObject)->TryGetNumberField(TEXT("x"), X) ||
        !(*PositionObject)->TryGetNumberField(TEXT("y"), Y) ||
        !(*PositionObject)->TryGetNumberField(TEXT("z"), Z))
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid position"));
        return false;
    }

    // Здоровье и валюта
    int32 Health = 0;
    int32 MaxHealth = 0;
    float Currency = 0.0f;
    (*PlayerObject)->TryGetNumberField(TEXT("health"), Health);
    (*PlayerObject)->TryGetNumberField(TEXT("max_health"), MaxHealth);
    (*PlayerObject)->TryGetNumberField(TEXT("currency"), Currency);

    UCurrentSessionData::SetHealth(Health);
    UCurrentSessionData::SetMaxHealth(MaxHealth);
    UCurrentSessionData::SetCurrency(Currency);
    UCurrentSessionData::SetSeed(Seed);

    return StartLevel(LevelNumber, Seed, FVector(X, Y, Z));
}

void UGameLoader::OnLevelLoaded(UWorld* LoadedWorld)
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(LevelLoadedHandle);
    LevelLoadedHandle.Reset();

    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(LoadedWorld, 0))
    {
        if (!LoadedPlayerPosition.IsZero())
        {
            PlayerPawn->SetActorLocation(LoadedPlayerPosition);
        }
    }

    LoadedPlayerPosition = FVector::ZeroVector;
}

#include "Serialization/JsonWriter.h"
#include "HAL/PlatformFilemanager.h"

bool UGameLoader::SaveGame(FString SavePath, int32 LevelNumber, int32 Seed, FVector PlayerPosition, int32 Health, int32 MaxHealth, float Currency)
{
    // Создаем JSON объект
    TSharedPtr<FJsonObject> RootObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> WorldObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> PlayerObject = MakeShareable(new FJsonObject);
    TSharedPtr<FJsonObject> PositionObject = MakeShareable(new FJsonObject);

    // Заполняем данные мира
    WorldObject->SetNumberField("level_number", LevelNumber);
    WorldObject->SetNumberField("seed", Seed);
    
    // Заполняем позицию игрока
    PositionObject->SetNumberField("x", PlayerPosition.X);
    PositionObject->SetNumberField("y", PlayerPosition.Y);
    PositionObject->SetNumberField("z", PlayerPosition.Z);
    
    // Заполняем данные игрока
    PlayerObject->SetObjectField("position", PositionObject);
    PlayerObject->SetNumberField("health", Health);
    PlayerObject->SetNumberField("max_health", MaxHealth);
    PlayerObject->SetNumberField("currency", Currency);

    // Собираем корневой объект
    RootObject->SetObjectField("world", WorldObject);
    RootObject->SetObjectField("player", PlayerObject);

    // Сериализуем в строку
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RootObject.ToSharedRef(), Writer);

    // Сохраняем в файл
    FString FullPath = FPaths::ProjectSavedDir() / TEXT("SaveGames/") + SavePath + ".json";
    return FFileHelper::SaveStringToFile(OutputString, *FullPath);
}

bool UGameLoader::SaveGame()
{
    FString SaveName = FString::Printf(TEXT("Slot%d"), UCurrentSessionData::GetSeed());
    
    UWorld* World = GWorld;
    if(!World) return false;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if(!PC) return false;

    APawn* PlayerPawn = PC->GetPawn();
    if(!PlayerPawn) return false;

    // Получение данных из сессии
    int32 LevelNumber = UCurrentSessionData::GetLevel();
    int32 Seed = UCurrentSessionData::GetSeed();
    int32 Health = UCurrentSessionData::GetHealth();
    int32 MaxHealth = UCurrentSessionData::GetMaxHealth();
    float Currency = UCurrentSessionData::GetCurrency();

    return SaveGame(
        SaveName,
        LevelNumber,
        Seed,
        PlayerPawn->GetActorLocation(),
        Health,
        MaxHealth,
        Currency
    );
}
