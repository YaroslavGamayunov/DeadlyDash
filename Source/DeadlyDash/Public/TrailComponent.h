// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TrailComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DEADLYDASH_API UTrailComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UTrailComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void BeginPlay() override;

	// Configuration properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	float TrailLifetime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	float MinDistanceBetweenPoints = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	UMaterialInterface* TrailMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	float TrailWidth = 10.0f;

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
	USplineComponent* TrailSpline;

	UPROPERTY()
	TArray<USplineMeshComponent*> SplineMeshes;

	UPROPERTY()
	TArray<FVector> TrailPoints;

	UPROPERTY()
	TArray<float> PointTimestamps;

	UPROPERTY()
	UMaterialInstanceDynamic* TrailMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	UStaticMesh* TrailMesh;

	bool bIsTrailActive;

	void UpdateSpline();
	void UpdateTrailMeshes();
	void CleanupOldPoints();
};