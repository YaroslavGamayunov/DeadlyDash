#pragma once
#include "FSaveSlotInfo.generated.h"

USTRUCT(BlueprintType)
struct FSaveSlotInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save System")
	FString SlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save System")
	FString FilePath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save System")
	FDateTime SaveTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save System")
	int32 Seed = 0;
};