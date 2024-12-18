#include "TrailComponent.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

UTrailComponent::UTrailComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Создаем компонент процедурного меша и регистрируем его
    TrailMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("TrailMeshComponent"));

    // Не прикрепляем меш к игроку, чтобы он рисовался относительно уровня
    TrailMeshComponent->SetupAttachment(nullptr);

    // Устанавливаем положение меша в нуль, чтобы он был в мировых координатах
    TrailMeshComponent->SetWorldLocation(FVector::ZeroVector);

    TrailMeshComponent->SetMobility(EComponentMobility::Movable);

    bIsTrailActive = false;
}

void UTrailComponent::BeginPlay()
{
    Super::BeginPlay();

    // Убеждаемся, что компонент меша зарегистрирован
    if (!TrailMeshComponent->IsRegistered())
    {
        TrailMeshComponent->RegisterComponent();
    }

    // Создаем динамический экземпляр материала
    if (TrailMaterial)
    {
        TrailMaterialInstance = UMaterialInstanceDynamic::Create(TrailMaterial, this);
        TrailMeshComponent->SetMaterial(0, TrailMaterialInstance);
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

    // Очищаем меш
    if (TrailMeshComponent)
    {
        TrailMeshComponent->ClearAllMeshSections();
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

        FVector CurrentLocation = GetComponentLocation();

        if (TrailPoints.Num() == 0 || FVector::Dist(CurrentLocation, TrailPoints.Last()) >= MinDistanceBetweenPoints)
        {
            // Добавляем новую точку
            TrailPoints.Add(CurrentLocation);
            PointTimestamps.Add(GetWorld()->GetTimeSeconds());

            UpdateTrailMeshes();
        }
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

// todo check performance (because mesh is recreated every tick)
void UTrailComponent::UpdateTrailMeshes()
{
    if (TrailPoints.Num() < 2 || !TrailMeshComponent)
    {
        return;
    }
    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    TArray<FProcMeshTangent> Tangents;

    // Половина ширины и высоты пути
    float TrailWidth = Width * 0.5f;
    float TrailHeight = Height * 0.5f;

    int32 NumPoints = TrailPoints.Num();

    for (int32 i = 0; i < NumPoints; ++i)
    {
        FVector CurrPoint = TrailPoints[i];

        FVector ForwardDir;
        if (i < NumPoints - 1)
        {
            ForwardDir = (TrailPoints[i + 1] - CurrPoint).GetSafeNormal();
        }
        else if (i > 0)
        {
            ForwardDir = (CurrPoint - TrailPoints[i - 1]).GetSafeNormal();
        }
        else
        {
            ForwardDir = FVector::ForwardVector;
        }

        FVector UpDir = FVector::UpVector; // Если "вверх" по оси Z
        FVector RightDir = FVector::CrossProduct(ForwardDir, UpDir).GetSafeNormal();

        // Четыре угла с учётом ширины и высоты
        FVector BottomLeft = CurrPoint - RightDir * TrailWidth - UpDir * TrailHeight;
        FVector BottomRight = CurrPoint + RightDir * TrailWidth - UpDir * TrailHeight;
        FVector TopLeft = CurrPoint - RightDir * TrailWidth + UpDir * TrailHeight;
        FVector TopRight = CurrPoint + RightDir * TrailWidth + UpDir * TrailHeight;

        // Добавляем вершины
        Vertices.Add(BottomLeft);   // Индекс: i*4 + 0
        Vertices.Add(BottomRight);  // Индекс: i*4 + 1
        Vertices.Add(TopLeft);      // Индекс: i*4 + 2
        Vertices.Add(TopRight);     // Индекс: i*4 + 3

        // Добавляем нормали для каждой вершины
        // Для упрощения можно использовать одну и ту же нормаль, но для правильного освещения рекомендуется вычислить нормали для каждой грани
        Normals.Add(-ForwardDir); // Нижние вершины
        Normals.Add(-ForwardDir);
        Normals.Add(ForwardDir);  // Верхние вершины
        Normals.Add(ForwardDir);

        // UV координаты
        float UCoord = i / (float)(NumPoints - 1);

        UVs.Add(FVector2D(UCoord, 0.0f)); // BottomLeft
        UVs.Add(FVector2D(UCoord, 1.0f)); // BottomRight
        UVs.Add(FVector2D(UCoord, 0.0f)); // TopLeft
        UVs.Add(FVector2D(UCoord, 1.0f)); // TopRight

        // Тангенты
        Tangents.Add(FProcMeshTangent(RightDir, false));
        Tangents.Add(FProcMeshTangent(RightDir, false));
        Tangents.Add(FProcMeshTangent(RightDir, false));
        Tangents.Add(FProcMeshTangent(RightDir, false));
    }

    // Создание треугольников для каждой секции пути
    for (int32 i = 0; i < NumPoints - 1; ++i)
    {
        int32 IndexBase = i * 4;

        // Индексы текущей и следующей точки
        int32 BL0 = IndexBase + 0; // BottomLeft текущий
        int32 BR0 = IndexBase + 1; // BottomRight текущий
        int32 TL0 = IndexBase + 2; // TopLeft текущий
        int32 TR0 = IndexBase + 3; // TopRight текущий

        int32 BL1 = IndexBase + 4; // BottomLeft следующий
        int32 BR1 = IndexBase + 5; // BottomRight следующий
        int32 TL1 = IndexBase + 6; // TopLeft следующий
        int32 TR1 = IndexBase + 7; // TopRight следующий

        // Верхняя грань
        Triangles.Add(TL0);
        Triangles.Add(TR1);
        Triangles.Add(TR0);

        Triangles.Add(TL0);
        Triangles.Add(TL1);
        Triangles.Add(TR1);

        // Нижняя грань
        Triangles.Add(BL0);
        Triangles.Add(BR0);
        Triangles.Add(BR1);

        Triangles.Add(BL0);
        Triangles.Add(BR1);
        Triangles.Add(BL1);

        // Левая грань
        Triangles.Add(BL0);
        Triangles.Add(TL1);
        Triangles.Add(TL0);

        Triangles.Add(BL0);
        Triangles.Add(BL1);
        Triangles.Add(TL1);

        // Правая грань
        Triangles.Add(BR0);
        Triangles.Add(TR0);
        Triangles.Add(TR1);

        Triangles.Add(BR0);
        Triangles.Add(TR1);
        Triangles.Add(BR1);

        // Передняя грань (по направлению движения)
        Triangles.Add(TL0);
        Triangles.Add(TR0);
        Triangles.Add(BR0);

        Triangles.Add(TL0);
        Triangles.Add(BR0);
        Triangles.Add(BL0);

        // Задняя грань
        Triangles.Add(TL1);
        Triangles.Add(BR1);
        Triangles.Add(TR1);

        Triangles.Add(TL1);
        Triangles.Add(BL1);
        Triangles.Add(BR1);
    }

    // Очищаем предыдущие секции меша
    TrailMeshComponent->ClearAllMeshSections();

    // Создаем новую секцию меша
    TrailMeshComponent->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FColor>(), Tangents, true);
    TrailMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // todo check

    // Устанавливаем материал
    if (TrailMaterialInstance)
    {
        TrailMeshComponent->SetMaterial(0, TrailMaterialInstance);
    }
}