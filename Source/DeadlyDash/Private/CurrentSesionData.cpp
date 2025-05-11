// Fill out your copyright notice in the Description page of Project Settings.


#include "CurrentSesionData.h"

int UCurrentSessionData::Seed = 0;
int UCurrentSessionData::Level = 1;
int UCurrentSessionData::Health = 100;
int UCurrentSessionData::MaxHealth = 100;
int UCurrentSessionData::Currency = 0;

TArray<FSaveSlotInfo> UCurrentSessionData::SavedSlotsList = TArray<FSaveSlotInfo>();

void UCurrentSessionData::SetSeed(int NewSeed)
{
	Seed = NewSeed;
}

int UCurrentSessionData::GetSeed()
{
	return Seed;
}

void UCurrentSessionData::SetCurrency(float NewCurrency)
{
	Currency = NewCurrency;
}

float UCurrentSessionData::GetCurrency()
{
	return Currency;
}

void UCurrentSessionData::SetHealth(int health)
{
	Health = health;
}

void UCurrentSessionData::SetMaxHealth(int health)
{
	MaxHealth = health;
}

int32 UCurrentSessionData::GetHealth()
{
	return Health;
}

int32 UCurrentSessionData::GetMaxHealth()
{
	return MaxHealth;
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

void UCurrentSessionData::SetLevel(int NewLevel)
{
	Level = NewLevel;
}


FString UCurrentSessionData::GetSessionDescription()
{
	return FString::Printf(
		TEXT("Level: %d\nSave Slot: %s\nSeed: %d"), 
		GetLevel(), 
		*GetSaveSlotName(), 
		GetSeed()
	);
}

