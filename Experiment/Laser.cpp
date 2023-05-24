
#include "Laser.h"
#include "DrawDebugHelpers.h"

#if 1
#define SLOG(t) GEngine->AddOnScreenDebugMessage(-1.f, 5.f, FColor::Green, t)
#define LINE(x, y, c) DrawDebugLine(GetWorld(), x, y, c, false, -1.f);
#define POINT(x, c) DrawDebugPoint(GetWorld(), x, 10.f, c, false, -1.f);
#else
#define SLOG(t)
#define LINE(x, y, c)
#define POINT(x, c)
#endif

ALaser::ALaser()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ALaser::BeginPlay()
{
	Super::BeginPlay();
}

void ALaser::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TArray<FHitResult> HitResults;
	FVector TraceEnd = GetActorLocation() - GetActorForwardVector() * LaserLength;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.bDebugQuery = true;
	Params.bFindInitialOverlaps = false;
	Params.bTraceComplex = true;
	Params.bIgnoreBlocks = false;
	Params.bSkipNarrowPhase = false;
	Params.IgnoreMask = true;

	bool bTraced = GetWorld()->LineTraceMultiByProfile(HitResults, GetActorLocation(), TraceEnd, "BlockAll", Params);

	FHitResult SingleHit;
	LINE(GetActorLocation(), TraceEnd, FColor::Cyan);
	
	GEngine->AddOnScreenDebugMessage(-1, -1.f, FColor::Red, FString::Printf(TEXT("Total Hits: %d"), HitResults.Num()));
	GEngine->AddOnScreenDebugMessage(-1, -1.f, FColor::Red, FString::Printf(TEXT("Traced: %d"), bTraced));

	if (HitResults.Num() > 0)
	{

		for (const FHitResult& Hit : HitResults)
		{
			if (Hit.bBlockingHit)
			{
				GEngine->AddOnScreenDebugMessage(1, 
					-1.f, 
					FColor::Red, 
					FString::Printf(TEXT("Actor Name: %s"), *Hit.GetActor()->GetActorNameOrLabel())
				);
				
				POINT(Hit.Location, FColor::Magenta);
			}
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, -1.f, FColor::Red, FString::Printf(TEXT("No Hit Results")));
	}
}

