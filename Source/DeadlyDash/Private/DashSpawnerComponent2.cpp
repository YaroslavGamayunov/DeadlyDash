#include "DashSpawnerComponent2.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/PlayerCameraManager.h"

UDashSpawnerComponent2::UDashSpawnerComponent2()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UDashSpawnerComponent2::BeginPlay()
{
	Super::BeginPlay();
	InitializeCubes();
	ScheduleSpawnCheck();
}

void UDashSpawnerComponent2::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
}

void UDashSpawnerComponent2::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ProcessArrowVisibility();
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

void UDashSpawnerComponent2::ScheduleSpawnCheck()
{
	GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, [this]()
	{
		MaintainArrows();
		ScheduleSpawnCheck();
	}, SpawnCheckInterval, false);
}

void UDashSpawnerComponent2::MaintainArrows()
{
	CleanupExpiredArrows();

	TSet<EDirection> VisibleDirections;
	for (const FArrowData& ArrowData : ActiveArrows)
	{
		if (ArrowData.bIsVisible && ArrowData.ArrowActor)
		{
			VisibleDirections.Add(ArrowData.Direction);
		}
	}

	const EDirection AllDirections[] = {EDirection::North, EDirection::East, EDirection::South, EDirection::West};
	for (EDirection Dir : AllDirections)
	{
		if (!VisibleDirections.Contains(Dir))
		{
			SpawnArrow(Dir);
		}
	}
}

void UDashSpawnerComponent2::CleanupExpiredArrows()
{
	auto world = GetWorld();
	float CurrentTime = 0.0f;
	if (world)
	{
		CurrentTime = world->GetTimeSeconds();
	}

	for (int32 i = ActiveArrows.Num() - 1; i >= 0; --i)
	{
		if (ActiveArrows[i].LastVisibleTime > 0.0f && CurrentTime - ActiveArrows[i].LastVisibleTime > ArrowLifetime)
		{
			DestroyArrow(ActiveArrows[i].ArrowActor);
			ReservedCubes.Remove(ActiveArrows[i].Cube);
			ActiveArrows.RemoveAt(i);
		}
	}
}

AActor* UDashSpawnerComponent2::GetSuitableCube() const
{
	TArray<AActor*> ValidCubes;
	for (AActor* Cube : AvailableCubes)
	{
		if (ReservedCubes.Contains(Cube)) continue;
		if (!IsCubeValid(Cube)) continue;
		ValidCubes.Add(Cube);
	}
	return ValidCubes.Num() > 0 ? ValidCubes[FMath::RandRange(0, ValidCubes.Num() - 1)] : nullptr;
}

bool UDashSpawnerComponent2::IsCubeValid(AActor* Cube) const
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

void UDashSpawnerComponent2::SpawnArrow(EDirection Direction)
{
	if (AActor* Cube = GetSuitableCube())
	{
		ReservedCubes.Add(Cube);
		const FTransform SpawnTransform(
			FRotator(0, static_cast<float>(Direction) * 90.0f, 0),
			Cube->GetActorLocation() + FVector(0, 0, 50)
		);

		AActor* NewArrow = GetWorld()->SpawnActor<AActor>(
			DashTriggerClass,
			SpawnTransform.GetLocation(),
			SpawnTransform.Rotator()
		);

		if (NewArrow)
		{
			ActiveArrows.Add({NewArrow, Cube, Direction, GetWorld()->GetTimeSeconds(), true});
		}
	}
}

void UDashSpawnerComponent2::DestroyArrow(AActor* Arrow)
{
	if (IsValid(Arrow)) Arrow->Destroy();
}

void UDashSpawnerComponent2::ProcessArrowVisibility()
{
	const float CurrentTime = GetWorld()->GetTimeSeconds();
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);

	if (!PC) return;

	for (FArrowData& ArrowData : ActiveArrows)
	{
		if (!IsValid(ArrowData.ArrowActor))
		{
			continue;
		}

		if (CurrentTime - ArrowData.LastVisibilityCheckTime < 0.1f)
		{
			continue;
		}

		ArrowData.LastVisibilityCheckTime = CurrentTime;

		FVector2D ScreenPosition;
		const bool bProjected = UGameplayStatics::ProjectWorldToScreen(
			PC,
			ArrowData.ArrowActor->GetActorLocation(),
			ScreenPosition,
			true
		);

		if (bProjected)
		{
			int32 ViewportX, ViewportY;
			PC->GetViewportSize(ViewportX, ViewportY);

			ArrowData.bIsVisible =
				ScreenPosition.X >= 0 && ScreenPosition.X <= ViewportX &&
				ScreenPosition.Y >= 0 && ScreenPosition.Y <= ViewportY &&
				(PC->PlayerCameraManager->GetCameraLocation() - ArrowData.ArrowActor->GetActorLocation()).Size() <
				5000.0f;
		}
		else
		{
			ArrowData.bIsVisible = false;
		}
		if (ArrowData.bIsVisible)
		{
			ArrowData.LastVisibleTime = CurrentTime;
		}
	}
}
