#include "TrailComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UTrailComponent::UTrailComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Создаем компонент процедурного меша и регистрируем его
    SplineComp = CreateDefaultSubobject<USplineComponent>(TEXT("TrailMeshComponent"));
    // Не прикрепляем меш к игроку, чтобы он рисовался относительно уровня
    SplineComp->SetupAttachment(nullptr);
    // Устанавливаем положение меша в нуль, чтобы он был в мировых координатах
    SplineComp->SetWorldLocation(FVector::ZeroVector);
    SplineComp->SetMobility(EComponentMobility::Movable);

    TrailStaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TrailStaticMeshComp"));

    bIsTrailActive = false;
}


void UTrailComponent::BeginPlay()
{
    Super::BeginPlay();

    // Убеждаемся, что компонент меша зарегистрирован
    if (!SplineComp->IsRegistered())
    {
        SplineComp->RegisterComponent();
    }
    if (!TrailStaticMeshComp->IsRegistered())
    {
        TrailStaticMeshComp->RegisterComponent();
    }

    TrailStaticMeshComp->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
 
    // Создаем динамический экземпляр материала
    if (TrailMaterial)
    {
        TrailMaterialInstance = UMaterialInstanceDynamic::Create(TrailMaterial, this);
        SplineComp->SetMaterial(0, TrailMaterialInstance);
        TrailMesh->SetMaterial(0, TrailMaterialInstance);
    }
}

void UTrailComponent::StartTrail()
{
    bIsTrailActive = true;

    // Очищаем предыдущие точки и меш
    ClearTrail();

    // Добавляем начальную точку
    FVector CurrentLocation = GetComponentLocation();
    TrailPoints.Add(CurrentLocation);
    PointTimestamps.Add(GetWorld()->GetTimeSeconds());

    UpdateTrailMeshes();
}

void UTrailComponent::StopTrail()
{
    bIsTrailActive = false;
}

void UTrailComponent::ClearTrail()
{
    TrailPoints.Empty();
    PointTimestamps.Empty();

    // todo 
    // Очищаем меш
    if (SplineComp)
    {
        SplineComp->ClearSplinePoints();
    }
}

void UTrailComponent::SetTrailColor(FLinearColor NewColor)
{
    if (TrailMaterialInstance)
    {
        TrailMaterialInstance->SetVectorParameterValue(FName("Color"), NewColor);
    }
}

void UTrailComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsTrailActive)
    {
        CleanupOldPoints();

        FVector CurrentLocation = GetProjectedPosition();

        if (TrailPoints.Num() == 0 || !IsCloseToLastPoint(CurrentLocation))
        {
            // Добавляем новую точку
            TrailPoints.Add(CurrentLocation);
            PointTimestamps.Add(GetWorld()->GetTimeSeconds());

            UpdateTrailMeshes();
        }
    }

    // todo OptimizeTrailPoints();
}

bool UTrailComponent::IsCloseToLastPoint(const FVector &PointToAdd) const
{
    if (TrailPoints.Num() == 0)
    {
        return false;
    }
    auto LastPoint = TrailPoints.Last();

    return FVector::DistXY(LastPoint, PointToAdd) < MinDistanceBetweenPoints;
}

float LinearityThreshold = 0.99f;

void UTrailComponent::OptimizeTrailPoints()
{
    if (TrailPoints.Num() < 3) return;

    TArray<FVector> NewPoints;

    int StraightLineStart = TrailPoints.Num() - 2;
    
    while (StraightLineStart > 0)
    {
        const FVector& PrevPoint = TrailPoints[StraightLineStart-1];
        const FVector& CurrentPoint = TrailPoints[StraightLineStart];
        const FVector& NextPoint = TrailPoints[StraightLineStart+1];
        
        // Проверяем коллинеарность трех точек
        FVector Dir1 = (CurrentPoint - PrevPoint).GetSafeNormal();
        FVector Dir2 = (NextPoint - CurrentPoint).GetSafeNormal();
        
        float Dot = FVector::DotProduct(Dir1, Dir2);
        
        // Если точки почти на одной прямой - удаляем среднюю точку
        if (Dot > LinearityThreshold)
        {
            StraightLineStart--;
        }
        else
        {
            break;
        }
    }

    if (StraightLineStart > 0)
    {
        TrailPoints.RemoveAt(StraightLineStart + 1, TrailPoints.Num() - StraightLineStart - 2);
    }
}

void UTrailComponent::CleanupOldPoints()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Удаляем точки, которые старше TrailLifetime
    while (PointTimestamps.Num() > 0 && (CurrentTime - PointTimestamps[0]) > TrailLifetime)
    {
        TrailPoints.RemoveAt(0);
        PointTimestamps.RemoveAt(0);

        // Перестраиваем меш после удаления старых точек
        UpdateTrailMeshes();
    }
}


FVector UTrailComponent::GetProjectedPosition() const
{
    FVector OriginalPosition = GetComponentLocation();
    
    
    USkeletalMeshComponent* PlayerMesh = Cast<USkeletalMeshComponent>(GetOwner()->FindComponentByClass<USkeletalMeshComponent>());
    
    if (PlayerMesh)
    {
        
        FBoxSphereBounds MeshBounds = PlayerMesh->Bounds;
        FVector LowestPoint = MeshBounds.Origin - MeshBounds.BoxExtent;
        
        return FVector(
            OriginalPosition.X,
            OriginalPosition.Y,
            LowestPoint.Z  
        );
    }
    
    return OriginalPosition;
}


// todo check performance (because mesh is recreated every tick)
void UTrailComponent::UpdateTrailMeshes()
{
    if (TrailPoints.Num() < 2 || !SplineComp)
    {
        return;
    }
    SplineComp->ClearSplinePoints(false);
    
    for (int32 i = 0; i < TrailPoints.Num(); i++)
    {
        // false для того, чтобы изменения копились и применялись оптом (опционально)
        SplineComp->AddSplinePoint(TrailPoints[i], ESplineCoordinateSpace::World, false);
            // Устанавливаем тип точки как Curve (или CurveClamped),
        // чтобы углы сглаживались автоматикой Unreal
        SplineComp->SetSplinePointType(i, ESplinePointType::Linear, false);
    }
    // Применяем изменения
    SplineComp->UpdateSpline();
    
    for (USplineMeshComponent* SplineMesh : SplineMeshPool)
    {
        if (SplineMesh)
        {
            SplineMesh->DestroyComponent();
        }
    }
    SplineMeshPool.Empty();

    // 5) Создаём новые SplineMeshComponents по парам соседних точек
    const int32 NumPoints = SplineComp->GetNumberOfSplinePoints();

    if (NumPoints > 1)
    {
        for (int32 i = 0; i < NumPoints - 1; i++)
        {
            // Создаем компонент, привязываем к нашему актеру
            USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
            if (SplineMesh)
            {
                SplineMesh->RegisterComponent();
                SplineMesh->SetMobility(EComponentMobility::Movable);
                SplineMesh->AttachToComponent(SplineComp, FAttachmentTransformRules::KeepRelativeTransform);

                // Применяем настройки коллизии
                SplineMesh->SetCollisionEnabled(CollisionEnabled);
                SplineMesh->SetCollisionProfileName(CollisionProfileName.Name, true);
                SplineMesh->SetCollisionObjectType(CollisionObjectType);

                // Устанавливаем ответы на коллизию
                for (const auto& Response : CollisionResponses)
                {
                    SplineMesh->SetCollisionResponseToChannel(Response.Key, Response.Value);
                }

                // Вытягиваем "начальную" точку и "конечную"
                FVector StartPos = SplineComp->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Local);
                FVector StartTan = SplineComp->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Local);

                FVector EndPos = SplineComp->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);
                FVector EndTan = SplineComp->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Local);

                // Задаём форму сегмента
                SplineMesh->SetStartAndEnd(StartPos, StartTan, EndPos, EndTan);
                SplineMesh->SetMaterial(0, TrailMaterialInstance);

                // todo
                if (TrailMesh)
                {
                    SplineMesh->SetStaticMesh(TrailMesh);
                }

                // Сохраняем в пул
                SplineMeshPool.Add(SplineMesh);
            }
        }
    }
}