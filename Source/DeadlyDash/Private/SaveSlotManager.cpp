// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSlotManager.h"

#include "FSaveSlotInfo.h"

TArray<FSaveSlotInfo> USaveSlotManager::GetSortedSaveSlots()
{
	TArray<FSaveSlotInfo> Slots;
	const FString SaveDir = FPaths::ProjectSavedDir() / TEXT("SaveGames/");
    
	// Создаем директорию если не существует
	IFileManager::Get().MakeDirectory(*SaveDir, true);

	TArray<FString> FileNames;
	IFileManager::Get().FindFiles(FileNames, *SaveDir, TEXT("Slot*.json"));

	for (const FString& FileName : FileNames)
	{
		int32 Seed;
		if (!ParseSlotName(FileName, Seed)) continue;

		FSaveSlotInfo SlotInfo;
		SlotInfo.SlotName = FString::Printf(TEXT("Slot%d"), Seed);
		SlotInfo.FilePath = SaveDir / FileName;
		SlotInfo.Seed = Seed;

		// Получаем время модификации файла
		FDateTime LastWriteTime = IFileManager::Get().GetTimeStamp(*SlotInfo.FilePath);
		SlotInfo.SaveTime = LastWriteTime;

		Slots.Add(SlotInfo);
	}

	// Сортировка по времени (новые сверху)
	Slots.Sort([](const FSaveSlotInfo& A, const FSaveSlotInfo& B) {
		return A.SaveTime > B.SaveTime;
	});

	return Slots;
}

bool USaveSlotManager::ParseSlotName(const FString& FileName, int32& OutSeed)
{
	const FString BaseName = FPaths::GetBaseFilename(FileName);
    
	// Проверяем формат: Slot{Number}.json
	if (!BaseName.StartsWith("Slot")) 
		return false;

	// Извлекаем числовую часть
	const FString SeedStr = BaseName.Mid(4, BaseName.Len() - 4);
    
	// Конвертируем с помощью Atoi
	OutSeed = FCString::Atoi(*SeedStr);
	return true;
}

FString USaveSlotManager::DateTimeToString(const FDateTime& DateTime)
{
	return DateTime.ToString();
};