// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ParkourCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;

UCLASS()
class AParkourCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AParkourCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	class UParkourMovementComponent* ParkourMovement;


protected:
	UPROPERTY(VisibleAnywhere, Category = "Spring Arm")
		USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
		UCameraComponent* FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Movement | Direction")
		bool bStrafeIsAllowed = true;

protected:
	virtual void BeginPlay() override;

#pragma region INPUT 

protected:
	virtual void MoveForward(float Value);

	virtual	void MoveRight(float Value);

	void Turn(float Value);

	void LookUp(float Value);

	virtual void ParkourJump();

	virtual void ParkourCrouch();

	virtual void Slide();

#pragma endregion

#pragma region Helpers
public:
	FCollisionQueryParams GetIgnoreCharacterParams() const;

private:
	FVector GetUnitVelocityDirection() const;

#pragma endregion

};
