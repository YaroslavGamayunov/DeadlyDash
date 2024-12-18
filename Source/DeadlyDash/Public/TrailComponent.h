#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TrailComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEADLYDASH_API UTrailComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UTrailComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	// Configuration properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	float TrailLifetime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	float MinDistanceBetweenPoints = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	UMaterialInterface* TrailMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	float Width = 100.0f; // Ширина полосы

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	float Height = 100.0f; // Высота полосы

	// Control functions
	UFUNCTION(BlueprintCallable, Category = "Trail")
	void StartTrail();

	UFUNCTION(BlueprintCallable, Category = "Trail")
	void StopTrail();

	UFUNCTION(BlueprintCallable, Category = "Trail")
	void ClearTrail();

	UFUNCTION(BlueprintCallable, Category = "Trail")
	void SetTrailColor(FLinearColor NewColor);

protected:
	UPROPERTY()
	TArray<FVector> TrailPoints;

	UPROPERTY()
	TArray<float> PointTimestamps;

	UPROPERTY()
	UMaterialInstanceDynamic* TrailMaterialInstance;

	UPROPERTY()
	UProceduralMeshComponent* TrailMeshComponent;

	bool bIsTrailActive;

	void UpdateTrailMeshes();
	void CleanupOldPoints();
};
