#include "DashSpawnerComponent2.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

UDashSpawnerComponent2::UDashSpawnerComponent2()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UDashSpawnerComponent2::BeginPlay()
{
    Super::BeginPlay();

    InitializeCubes();
    SpawnInitialArrows();
    
    GetWorld()->GetTimerManager().SetTimer(
        ViewportCheckTimer,
        this,
        &UDashSpawnerComponent2::CheckViewportAndSpawn,
        ViewportCheckInterval,
        true
    );
}

void UDashSpawnerComponent2::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    for (AActor* Arrow : ActiveArrows)
    {
        if (IsValid(Arrow)) Arrow->Destroy();
    }
    ActiveArrows.Empty();
}

void UDashSpawnerComponent2::InitializeCubes()
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        // Проверка пути к актору в редакторе
        FString ActorPath = Actor->GetPathName(GetWorld());
        if (ActorPath.Contains(CubeFolderName.ToString()))
        {
            AvailableCubes.Add(Actor);
        }
    }
}

void UDashSpawnerComponent2::SpawnInitialArrows()
{
    const TArray<EDirection> InitialDirections = {
        EDirection::North,
        EDirection::East,
        EDirection::South,
        EDirection::West
    };

    for (const EDirection Dir : InitialDirections)
    {
        SpawnArrow(Dir);
    }
}

bool UDashSpawnerComponent2::IsArrowVisible(AActor* Arrow) const
{
    if (!IsValid(Arrow)) return false;

    APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
    if (!CameraManager) return false;

    FVector CameraLocation = CameraManager->GetCameraLocation();
    FVector ArrowLocation = Arrow->GetActorLocation();

    // Проверка через проекцию на viewport
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return false;

    FVector2D ScreenPosition;
    PC->ProjectWorldLocationToScreen(ArrowLocation, ScreenPosition, true);
    
    int32 ViewportX, ViewportY;
    PC->GetViewportSize(ViewportX, ViewportY);
    
    return ScreenPosition.X >= 0 && ScreenPosition.X <= ViewportX &&
           ScreenPosition.Y >= 0 && ScreenPosition.Y <= ViewportY;
}

EDirection UDashSpawnerComponent2::GetArrowDirection(AActor* Arrow) const
{
    if (!IsValid(Arrow)) return EDirection::North;
    
    const float Yaw = Arrow->GetActorRotation().Yaw;
    const int32 DirectionIndex = FMath::RoundToInt(Yaw / 90.0f) % 4;
    return static_cast<EDirection>(DirectionIndex);
}

void UDashSpawnerComponent2::CheckViewportAndSpawn()
{
    // Удаляем невидимые стрелки
    TArray<AActor*> ArrowsToRemove;
    for (AActor* Arrow : ActiveArrows)
    {
        if (!IsArrowVisible(Arrow))
        {
            ArrowsToRemove.Add(Arrow);
        }
    }

    for (AActor* Arrow : ArrowsToRemove)
    {
        DestroyArrow(Arrow);
    }

    // Поддерживаем общее количество
    MaintainArrowCount();
}

EDirection UDashSpawnerComponent2::GetRandomMissingDirection() const
{
    TSet<EDirection> ExistingDirections;
    for (AActor* Arrow : ActiveArrows)
    {
        ExistingDirections.Add(GetArrowDirection(Arrow));
    }

    TArray<EDirection> PossibleDirections;
    const TArray<EDirection> AllDirections = {
        EDirection::North,
        EDirection::East,
        EDirection::South,
        EDirection::West
    };

    for (const EDirection Dir : AllDirections)
    {
        if(!ExistingDirections.Contains(Dir) || 
          (ExistingDirections.Num() == AllDirections.Num()))
        {
            PossibleDirections.Add(Dir);
        }
    }

    return PossibleDirections.Num() > 0 ? 
        PossibleDirections[FMath::RandRange(0, PossibleDirections.Num()-1)] : 
        AllDirections[FMath::RandRange(0, 3)];
}


void UDashSpawnerComponent2::MaintainArrowCount()
{
    const int32 TargetCount = 4;
    const int32 CurrentCount = ActiveArrows.Num();
    
    if(CurrentCount < TargetCount)
    {
        // Создаем недостающие стрелки
        for(int32 i = 0; i < TargetCount - CurrentCount; ++i)
        {
            SpawnArrow(GetRandomMissingDirection());
        }
    }
}

void UDashSpawnerComponent2::SpawnArrow(EDirection Direction)
{
    if(ActiveArrows.Num() >= 4) return; // Дополнительная проверка

    AActor* Cube = GetRandomFreeCube();
    if (!Cube || !DashTriggerClass) return;

    const FRotator SpawnRotation(0, static_cast<int32>(Direction) * 90.0f, 0);
    const FVector SpawnLocation = Cube->GetActorLocation() + FVector(0, 0, 50);

    if (AActor* NewArrow = GetWorld()->SpawnActor<AActor>(DashTriggerClass, SpawnLocation, SpawnRotation))
    {
        ActiveArrows.Add(NewArrow);

        FTimerHandle LifeTimer;
        FTimerDelegate Delegate;
        Delegate.BindUObject(this, &UDashSpawnerComponent2::DestroyArrow, NewArrow);
        GetWorld()->GetTimerManager().SetTimer(LifeTimer, Delegate, ArrowLifeTime, false);
    }
}

void UDashSpawnerComponent2::DestroyArrow(AActor* Arrow)
{
    if (IsValid(Arrow))
    {
        ActiveArrows.Remove(Arrow);
        Arrow->Destroy();
        
        // Немедленно создаем новую стрелку при удалении
        MaintainArrowCount();
    }
}

AActor* UDashSpawnerComponent2::GetRandomFreeCube() const
{
    TArray<AActor*> ValidCubes;

    for (AActor* Cube : AvailableCubes)
    {
        if (!IsCubeValidForSpawning(Cube)) continue;
        
        bool bIsOccupied = false;
        for (AActor* Arrow : ActiveArrows)
        {
            if (FVector::DistSquared(Cube->GetActorLocation(), 
                Arrow->GetActorLocation()) < 100.0f)
            {
                bIsOccupied = true;
                break;
            }
        }
        
        if (!bIsOccupied) ValidCubes.Add(Cube);
    }

    return ValidCubes.Num() > 0 ? 
        ValidCubes[FMath::RandRange(0, ValidCubes.Num() - 1)] : nullptr;
}

bool UDashSpawnerComponent2::IsCubeValidForSpawning(AActor* Cube) const
{
    if (!IsValid(Cube)) return false;

    // Проверка дистанции до игрока
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return true;

    const FVector PlayerLocation = PlayerPawn->GetActorLocation();
    const FVector CubeLocation = Cube->GetActorLocation();
    
    // Проверка горизонтального расстояния
    const FVector2D PlayerPos2D(PlayerLocation.X, PlayerLocation.Y);
    const FVector2D CubePos2D(CubeLocation.X, CubeLocation.Y);
    const float HorizontalDist = FVector2D::Distance(PlayerPos2D, CubePos2D);
    
    if (HorizontalDist < MinDistanceToPlayer) return false;

    // Проверка видимости куба
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return true;

    FVector2D ScreenPosition;
    PC->ProjectWorldLocationToScreen(CubeLocation, ScreenPosition, true);
    
    int32 ViewportX, ViewportY;
    PC->GetViewportSize(ViewportX, ViewportY);
    
    return ScreenPosition.X >= 0 && ScreenPosition.X <= ViewportX &&
           ScreenPosition.Y >= 0 && ScreenPosition.Y <= ViewportY;
}