// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ParkourCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;

UENUM(BlueprintType)
enum class EJumpLandType : uint8
{
	EJLT_None	UMETA(DisplayName = "None"),
	EJLT_Normal UMETA(DisplayName = "Normal Land"),
	EJLT_Hard	UMETA(DisplayName = "Hard Land"),
	EJLT_Roll	UMETA(DisplayName = "Roll"),
	EJLT_Max    UMETA(DisplayName = "MAX")
};


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
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Spring Arm")
		USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
		UCameraComponent* FollowCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Movement | Direction")
		bool bStrafeIsAllowed = true;

public:
	// Animation Data Helpers
	UPROPERTY(BlueprintReadOnly, Category = "Animation | Falling")
		EJumpLandType LandType = EJumpLandType::EJLT_None;

	UPROPERTY(EditDefaultsOnly, Category = "Movement | Fall Speed Squared")
		float HardLandMinSpeed = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement | Fall Speed Squared")
		float RollLandMinSpeed = 1000.f;
#pragma region INPUT 

protected:
	virtual void MoveForward(float Value);
	virtual	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	virtual void ParkourJump();
	virtual void ParkourCrouch();
	virtual void Slide();
	virtual void DashPressed();
	virtual void DashReleased();

#pragma endregion

#pragma region Helpers
public:
	FCollisionQueryParams GetIgnoreCharacterParams() const;

private:
	FVector GetUnitVelocityDirection() const;

#pragma endregion

};
