// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CollisionQueryParams.h"
#include "Laser.generated.h"

UCLASS()
class PARKOUR_API ALaser : public AActor
{
	GENERATED_BODY()
	
public:	
	ALaser();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Laser")
		float LaserLength = 100.f;

	/*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Laser")
		FCollisionQueryParams BPparams;*/

};
