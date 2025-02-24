#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	bool IsCurved = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trail")
	UStaticMesh* TrailMesh;

	// Collision Preset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	FCollisionProfileName CollisionProfileName = FName(TEXT("BlockAll"));

	// Collision Enabled
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled = ECollisionEnabled::QueryAndPhysics;

	// Object Type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TEnumAsByte<ECollisionChannel> CollisionObjectType = ECC_WorldStatic;

	// Collision Responses
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
	TMap<TEnumAsByte<ECollisionChannel>, TEnumAsByte<ECollisionResponse>> CollisionResponses;
	
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Trail")
	USplineComponent* SplineComp;

	UStaticMeshComponent* TrailStaticMeshComp;

	// Массив точек, по которым построим сплайн
	UPROPERTY()
	TArray<FVector> TrailPoints;

	UPROPERTY()
	TArray<float> PointTimestamps;

	// Храним ссылки на созданные SplineMeshComponents, чтобы при обновлении их чистить
	TArray<USplineMeshComponent*> SplineMeshPool;

	UPROPERTY()
	UMaterialInstanceDynamic* TrailMaterialInstance;

	bool bIsTrailActive;

	FVector GetProjectedPosition() const;

	bool IsCloseToLastPoint(const FVector &PointToAdd) const;

	void UpdateTrailMeshes();
	void CleanupOldPoints();
	void OptimizeTrailPoints();
};
