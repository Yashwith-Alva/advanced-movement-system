// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ParkourMovementComponent.generated.h"

#define OUT
#define bDrawDebug 1

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDashStartDelegate);

/* Custom Movement Mode */
UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None			UMETA(Hidden),
	CMOVE_Slide			UMETA(DisplayName = "Slide"),
	CMOVE_WallRun		UMETA(DisplayName = "Wall Run"),
	CMOVE_Hang			UMETA(DisplayName = "Hang"),
	CMOVE_Climb			UMETA(DisplayName = "Climb"),
	CMOVE_MAX			UMETA(Hidden),
};

/* Object Info Struct */
USTRUCT(BlueprintType)
struct FWallInfo {
	GENERATED_USTRUCT_BODY()

public:	
	UPROPERTY(VisibleAnywhere)
		float WallDistance = 5.f;
	
	UPROPERTY(VisibleAnywhere)
		float wallHeight = 0.f;
	
	UPROPERTY(VisibleAnywhere)
		float wallWidth = 0.f;

	UPROPERTY(VisibleAnywhere)
		float wallAngleSteep = 0.f;
};


/**
 * Advanced Character Movement with Multiplayer functionality
 */
UCLASS()
class UParkourMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

private:

	typedef UCharacterMovementComponent Super;

	/* Saved Move used to send additional information through network.*/
	class FSavedMove_Parkour : public FSavedMove_Character
	{
	public:
		enum CompressedFlags
		{
			FLAG_Dash = 0x10,
			FLAG_Custom_1 = 0x20,
			FLAG_Custom_2 = 0x40,
			FLAG_Custom_3 = 0x80
		};

		// FLAGS
		uint8 Saved_bPressedParkourJump : 1;
		uint8 Saved_bWantsToDash : 1;

		// Other Variables
		uint8 Saved_bHadAnimRootMotion : 1;
		uint8 Saved_bTransitionFinished : 1;
		uint8 Saved_bWantsToSlide : 1;
		uint8 Saved_bWallRunIsRight : 1;

		FSavedMove_Parkour();

		/* Clear saved move properties, so it can be re-used. */
		virtual void Clear() override;

		/* Called to set up this saved move (when initially created) to make a predictive correction. */
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;

		/* Returns true if this move can be combined with NewMove for replication without changing any behavior */
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;

		/* Called before ClientUpdatePosition uses this SavedMove to make a predictive correction */
		virtual void PrepMoveFor(ACharacter* C) override;

		/* Returns a byte containing encoded special movement information (jumping, crouching, etc.) */
		virtual uint8 GetCompressedFlags() const override;

	};

	/* Prediction data */
	class FNetworkPredictionData_Client_Parkour : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_Parkour(const UCharacterMovementComponent& ClientMovement);

		typedef FNetworkPredictionData_Client_Character Super;

		virtual FSavedMovePtr AllocateNewMove() override;

	};

#pragma region Parameters

	// Sliding
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement |Slide") float MinSlideSpeed = 400.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement |Slide") float MaxSlideSpeed = 400.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement |Slide") float SlideEnterImpulse = 400.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement |Slide") float SlideGravityForce = 4000.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement |Slide") float SlideFrictionFactor = 0.6f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement |Slide") float BreakingDecelerationSliding = 1000.f;

	// Dash
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement |Dash") float DashCooldownDuration = 1.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement |Dash") float AuthDashCooldownDuration = 0.9f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement |Dash") UAnimMontage* DashMontage;

	// Wall Run
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement | Wall Run") float MinWallRunSpeed = 200.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement | Wall Run") float MaxWallRunSpeed = 800.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement | Wall Run") float MaxVerticalWallRunSpeed = 200.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement | Wall Run") float WallRunPullAwayAngle = 75;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement | Wall Run") float WallAttractionForce = 200.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement | Wall Run") float MinWallRunHeight = 50.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement | Wall Run") UCurveFloat* WallRunGravityScaleCurve;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement | Wall Run") float WallJumpOffForce = 300.f;

	// Wall Details
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement | Wall Movement") float WallDistance = 100.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement | Wall Movement") float MaxWallHeight = 200.f;
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement | Wall Movement") float MaxWallWidth = 250.f;
	
	// Debuggers
	UPROPERTY(EditDefaultsOnly, Category = "Parkour Movement | Debug Movement") bool bWallDebug = false;

#pragma endregion

#pragma region Transient
	UPROPERTY(Transient) class AParkourCharacter* ParkourCharacterOwner;

	// Flags
	bool Safe_bWantsToDash;

	bool Safe_bHadAnimRootMotion;
	bool Safe_bWantsToSlide;

	float DashStartTime;
	FTimerHandle TimerHandle_DashCooldown;
	FDashStartDelegate DashStartDelegate;

	bool Safe_bTransitionFinished;
	TSharedPtr<FRootMotionSource_MoveToForce> TransitionRMS;
	FString TransitionName;

	UPROPERTY(Transient) UAnimMontage* TransitionQueuedMontage;
	float TransitionQueuedMontageSpeed;
	int TransitionRMS_ID;

	bool Safe_bWallRunIsRight;
#pragma endregion

	// Replication
	UPROPERTY(ReplicatedUsing = OnRep_Dash) bool Proxy_bDash;


public:
	// Constructor if Required
	UParkourMovementComponent();


protected:
	virtual void InitializeComponent();

#pragma region Derived From Actor Component
protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

public:
	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;
	virtual bool IsMovingOnGround() const override;
	virtual bool CanCrouchInCurrentState() const override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;

	virtual bool CanAttemptJump() const override;
	virtual bool DoJump(bool bReplayingMoves) override;

public:
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

protected:
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

#pragma endregion

private:
#pragma region MOVEMENT PHYS

	// Slide
	void EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode);
	void ExitSlide();
	bool CanSlide() const;
	void PhysSlide(float deltaTime, int32 Iterations);

	// Dash
	void OnDashCooldownFinished();
	bool CanDash() const;
	void PerformDash();

	// WallRun
	bool TryWallRun();
	void PhysWallRun(float deltaTime, int32 Iterations);

#pragma endregion

	// Helpers
	bool IsServer() const;
	float CapR() const;
	float CapHH() const;
	void GetWallDetails(OUT FWallInfo* WallDetails);

#pragma region Interface
public:
	UFUNCTION(BlueprintCallable) void DashPressed();
	UFUNCTION(BlueprintCallable) void DashReleased();

	UFUNCTION(BlueprintPure) bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
	UFUNCTION(BlueprintPure) bool IsMovementMode(EMovementMode InMovementMode) const;

	UFUNCTION(BlueprintPure) bool IsWallRunning() const { return IsCustomMovementMode(CMOVE_WallRun); }
	UFUNCTION(BlueprintPure) bool WallRunningIsRight() const { return Safe_bWallRunIsRight; }

#pragma endregion

#pragma region Proxy Replication
public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UFUNCTION() void OnRep_Dash();

#pragma endregion


#pragma region DEBUGGERS
	UPROPERTY(EditDefaultsOnly, Category = "Debugging")
		bool bDebugSlide;
#pragma endregion
};
