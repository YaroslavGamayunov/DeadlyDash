// Fill out your copyright notice in the Description page of Project Settings.


#include "CurrentSesionData.h"

int UCurrentSessionData::Seed = 0;
int UCurrentSessionData::Level = 0;
TArray<FSaveSlotInfo> UCurrentSessionData::SavedSlotsList = TArray<FSaveSlotInfo>();

void UCurrentSessionData::SetSeed(int NewSeed)
{
	Seed = NewSeed;
}

int UCurrentSessionData::GetSeed()
{
	return Seed;
}

FString UCurrentSessionData::GetSaveSlotName()
{
	return FString::Printf(TEXT("Slot%d"), Seed);
}

void UCurrentSessionData::SetSavedSlotsList(const TArray<FSaveSlotInfo> &slots)
{
	SavedSlotsList = slots;
}

TArray<FSaveSlotInfo> UCurrentSessionData::GetSavedSlotsList()
{
	return SavedSlotsList;
}

int UCurrentSessionData::GetLevel()
{
	return Level;
}

int UCurrentSessionData::IncreaseLevel()
{
	Level++;
	return Level;
}