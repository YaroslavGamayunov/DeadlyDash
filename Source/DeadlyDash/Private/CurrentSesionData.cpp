// Fill out your copyright notice in the Description page of Project Settings.


#include "CurrentSesionData.h"

int32 UCurrentSessionData::Seed = 0;
TArray<FSaveSlotInfo> UCurrentSessionData::SavedSlotsList = TArray<FSaveSlotInfo>();

void UCurrentSessionData::SetSeed(int32 NewSeed)
{
	Seed = NewSeed;
}

int32 UCurrentSessionData::GetSeed()
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