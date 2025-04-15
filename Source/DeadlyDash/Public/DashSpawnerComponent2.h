#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DashSpawnerComponent2.generated.h"

UENUM(BlueprintType)
enum class EDirection : uint8
{
	North,
	East,
	South,
	West
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEADLYDASH_API UDashSpawnerComponent2 : public UActorComponent
{
	GENERATED_BODY()

public:
	UDashSpawnerComponent2();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// Конфигурация
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	TSubclassOf<AActor> DashTriggerClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	FName CubeFolderName = TEXT("CubeS"); // Имя папки в редакторе

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	float ArrowLifeTime = 30.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Spawning")
	float ViewportCheckInterval = 0.5f;
	
	// Состояние
	TArray<AActor*> AvailableCubes;
	TArray<AActor*> ActiveArrows;
	FTimerHandle ViewportCheckTimer;
	UPROPERTY(EditDefaultsOnly, Category="Spawning")
	float MinDistanceToPlayer = 100.0f;

	bool IsCubeValidForSpawning(AActor* Cube) const;
	void InitializeCubes();
	void SpawnInitialArrows();
	void CheckViewportAndSpawn();
	bool IsArrowVisible(AActor* Arrow) const;
	EDirection GetArrowDirection(AActor* Arrow) const;
	void SpawnArrow(EDirection Direction);
	AActor* GetRandomFreeCube() const;
	EDirection GetRandomMissingDirection() const;
	void DestroyArrow(AActor* Arrow);
	void MaintainArrowCount();
	void ReplaceArrow(AActor* ArrowToReplace);
};