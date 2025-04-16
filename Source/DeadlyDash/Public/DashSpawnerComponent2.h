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

USTRUCT()
struct FArrowData
{
	GENERATED_BODY()

	UPROPERTY()
	AActor* ArrowActor = nullptr;

	UPROPERTY()
	AActor* Cube = nullptr;

	UPROPERTY()
	EDirection Direction;
    
	double LastVisibilityCheckTime = 0.0f;
	double LastVisibleTime = -1.0f;
	bool bIsVisible = false;
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
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// Конфигурация
	UPROPERTY(EditDefaultsOnly) TSubclassOf<AActor> DashTriggerClass;
	UPROPERTY(EditDefaultsOnly) float ArrowLifetime = 5.0f;
	UPROPERTY(EditDefaultsOnly) float SpawnCheckInterval = 0.2f;
	UPROPERTY(EditDefaultsOnly)
	FName CubeFolderName = TEXT("CubeS"); // Имя папки в редакторе
    
	// Состояние
	TArray<AActor*> AvailableCubes;
	TArray<FArrowData> ActiveArrows;
	TSet<AActor*> ReservedCubes;
	FTimerHandle SpawnTimerHandle;
	float MinDistanceToPlayer = 100.0f;

	void InitializeCubes();
	void ScheduleSpawnCheck();
	void ProcessArrowVisibility();
	void MaintainArrows();
	void SpawnArrow(EDirection Direction);
	AActor* GetSuitableCube() const;
	bool IsCubeValid(AActor* Cube) const;
	void CleanupExpiredArrows();
	void DestroyArrow(AActor* Arrow);
};