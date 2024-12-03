// Fill out your copyright notice in the Description page of Project Settings.


#include "TrailComponent.h"
#include "Components/SplineComponent.h"
#include "Engine/World.h"
#include "Logging/StructuredLog.h"
#include "Materials/MaterialInstanceDynamic.h"

UTrailComponent::UTrailComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Create spline component
    TrailSpline = CreateDefaultSubobject<USplineComponent>(TEXT("TrailSpline"));
    TrailSpline->SetupAttachment(this);
}

void UTrailComponent::BeginPlay()
{
    Super::BeginPlay();

    if (TrailMaterial)
    {
        TrailMaterialInstance = UMaterialInstanceDynamic::Create(TrailMaterial, this);
    }

    bIsTrailActive = false;
}

void UTrailComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsTrailActive) return;

    // Get current location
    FVector CurrentLocation = GetComponentLocation();

    // Add new point if needed
    if (TrailPoints.Num() == 0 || 
        FVector::Distance(TrailPoints.Last(), CurrentLocation) > MinDistanceBetweenPoints)
    {
        TrailPoints.Add(CurrentLocation);
        PointTimestamps.Add(GetWorld()->GetTimeSeconds());
        UpdateSpline();
    }

    CleanupOldPoints();
}

void UTrailComponent::StartTrail()
{
    bIsTrailActive = true;
}

void UTrailComponent::StopTrail()
{
    bIsTrailActive = false;
}

void UTrailComponent::ClearTrail()
{
    TrailPoints.Empty();
    PointTimestamps.Empty();
    
    // Clean up spline meshes
    for (auto* Mesh : SplineMeshes)
    {
        if (Mesh)
        {
            Mesh->DestroyComponent();
        }
    }
    SplineMeshes.Empty();
    
    TrailSpline->ClearSplinePoints();
}

void UTrailComponent::SetTrailColor(FLinearColor NewColor)
{
    if (TrailMaterialInstance)
    {
        TrailMaterialInstance->SetVectorParameterValue("Color", NewColor);
    }
}

void UTrailComponent::UpdateSpline()
{
    TrailSpline->ClearSplinePoints();
    
    for (int32 i = 0; i < TrailPoints.Num(); i++)
    {
        TrailSpline->AddSplinePoint(TrailPoints[i], ESplineCoordinateSpace::World);
    }
    
    UpdateTrailMeshes();
}

void UTrailComponent::UpdateTrailMeshes()
{
    // Clean up old meshes
    for (auto* Mesh : SplineMeshes)
    {
        if (Mesh)
        {
            Mesh->DestroyComponent();
        }
    }
    SplineMeshes.Empty();

    UE_LOGFMT(LogCore, Warning, "Loading `{Name}` failed with error {Error}", Package->GetName(),  ErrorCode);

    // Create new meshes
    for (int32 i = 0; i < TrailPoints.Num() - 1; i++)
    {
        USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
        SplineMesh->SetMobility(EComponentMobility::Movable);
        SplineMesh->AttachToComponent(TrailSpline, FAttachmentTransformRules::KeepRelativeTransform);
        SplineMesh->SetStaticMesh(TrailMesh);
        SplineMesh->SetMaterial(0, TrailMaterialInstance);
        
        FVector StartPoint, StartTangent, EndPoint, EndTangent;
        TrailSpline->GetLocationAndTangentAtSplinePoint(i, StartPoint, StartTangent, ESplineCoordinateSpace::World);
        TrailSpline->GetLocationAndTangentAtSplinePoint(i + 1, EndPoint, EndTangent, ESplineCoordinateSpace::World);
        
        SplineMesh->SetStartAndEnd(StartPoint, StartTangent, EndPoint, EndTangent, true);
        SplineMesh->SetStartScale(FVector2D(TrailWidth, TrailWidth));
        SplineMesh->SetEndScale(FVector2D(TrailWidth, TrailWidth));
        
        SplineMesh->RegisterComponent();
        SplineMeshes.Add(SplineMesh);
    }
}

void UTrailComponent::CleanupOldPoints()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    while (PointTimestamps.Num() > 0 && 
           (CurrentTime - PointTimestamps[0]) > TrailLifetime)
    {
        PointTimestamps.RemoveAt(0);
        TrailPoints.RemoveAt(0);
    }
    
    if (TrailPoints.Num() > 0)
    {
        UpdateSpline();
    }
}