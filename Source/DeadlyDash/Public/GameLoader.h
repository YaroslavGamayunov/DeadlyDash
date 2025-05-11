#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameLoader.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnLevelLoadedSignature, UWorld*, LoadedWorld);

UCLASS()
class DEADLYDASH_API UGameLoader : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "File Operations", meta = (DisplayName = "Load JSON And Start Level"))
	static bool LoadJSONAndStartLevel(FString Path);
    
	UFUNCTION(BlueprintCallable, Category = "Game Loader", meta = (DisplayName = "Start Level"))
	static bool StartLevel(int32 level, int32 seed, FVector position);
	
	static bool SaveGame(FString SavePath, int32 LevelNumber, int32 Seed, FVector PlayerPosition, int32 Health, int32 MaxHealth,  float Currency);

	UFUNCTION(BlueprintCallable, Category = "Game Loader", meta = (DisplayName = "Save Game"))
	static bool SaveGame();

private:
	static FVector LoadedPlayerPosition;
	static FDelegateHandle LevelLoadedHandle;
	static void OnLevelLoaded(UWorld* LoadedWorld);
};