// Fill out your copyright notice in the Description page of Project Settings.


#include "ParkourMovementComponent.h"
#include "Parkour/Character/ParkourCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

// Helper MACROS
#if 1
float MacroDuration = 8.f;
#define DLOG(x, c) GEngine->AddOnScreenDebugMessage(-1, MacroDuration ? MacroDuration : -1.f, c, x)
#define BOX(x, y, c) DrawDebugBox(GetWorld(), x, y, c, !MacroDuration, MacroDuration);
#define LINE(x1, x2, c) DrawDebugLine(GetWorld(), x1, x2, c, !MacroDuration, MacroDuration);
#define ARROW(x, y, d, c) DrawDebugDirectionalArrow(GetWorld(), x, y, d, c, false, MacroDuration);
#define POINT(x, c) DrawDebugPoint(GetWorld(), x, 10, c, !MacroDuration, MacroDuration);
#define CAPSULE(x, c) DrawDebugCapsule(GetWorld(), x, CapHH(), CapR(), FQuat::Identity, c, !MacroDuration, MacroDuration);
#define BOXROTATE(x, y, r, c) DrawDebugBox(GetWorld(), x, y, r, c, false, MacroDuration);
#else
#define DLOG(x, c)
#define BOX(x, y, c)
#define LINE(x1, x2, c)
#define ARROW(x, y, d, c)
#define POINT(x, c)
#define CAPSULE(x, c)
#define BOXROTATE(x, y, r, c)
#endif

#define SLOG(x) GEngine->AddOnScreenDebugMessage(-1, 5.f ? 5.f : -1.f, FColor::Green, x);

#if 0
	#define DRAW_CLEARANCE
#endif

#pragma region SAVED MOVE
UParkourMovementComponent::FSavedMove_Parkour::FSavedMove_Parkour()
{
	Saved_bWantsToSlide = 0;
}

void UParkourMovementComponent::FSavedMove_Parkour::Clear()
{
	FSavedMove_Character::Clear();

	Saved_bWantsToDash = 0;
	Saved_bWantsToSlide = 0;
	Saved_bPressedParkourJump = 0;

	Saved_bHadAnimRootMotion = 0;
	Saved_bTransitionFinished = 0;

	Saved_bWallRunIsRight = 0;

}

void UParkourMovementComponent::FSavedMove_Parkour::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	FSavedMove_Character::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	const UParkourMovementComponent* CharacterMovement = Cast<UParkourMovementComponent>(C->GetCharacterMovement());

	Saved_bWantsToDash = CharacterMovement->Safe_bWantsToDash;
	Saved_bWantsToSlide = CharacterMovement->Safe_bWantsToSlide;
	Saved_bPressedParkourJump = CharacterMovement->ParkourCharacterOwner->bPressedJump;

	Saved_bHadAnimRootMotion = CharacterMovement->Safe_bHadAnimRootMotion;
	Saved_bTransitionFinished = CharacterMovement->Safe_bTransitionFinished;

	Saved_bWallRunIsRight = CharacterMovement->Safe_bWallRunIsRight;

}

bool UParkourMovementComponent::FSavedMove_Parkour::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	const FSavedMove_Parkour* NewParkourMove = static_cast<FSavedMove_Parkour*>(NewMove.Get());

	if (Saved_bWallRunIsRight != NewParkourMove->Saved_bWallRunIsRight)
		return false;

	if (Saved_bWantsToDash != NewParkourMove->Saved_bWantsToDash)
		return false;

	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UParkourMovementComponent::FSavedMove_Parkour::PrepMoveFor(ACharacter* C)
{
	FSavedMove_Character::PrepMoveFor(C);

	UParkourMovementComponent* CharacterMovement = Cast<UParkourMovementComponent>(C->GetCharacterMovement());

	CharacterMovement->Safe_bWantsToSlide = Saved_bWantsToSlide;
	CharacterMovement->Safe_bWantsToDash = Saved_bWantsToDash;
	CharacterMovement->ParkourCharacterOwner->bPressedJump = Saved_bPressedParkourJump;

	CharacterMovement->Safe_bHadAnimRootMotion = Saved_bHadAnimRootMotion;
	CharacterMovement->Safe_bTransitionFinished = Saved_bTransitionFinished;

	CharacterMovement->Safe_bWallRunIsRight = Saved_bWallRunIsRight;

}

uint8 UParkourMovementComponent::FSavedMove_Parkour::GetCompressedFlags() const
{
	uint8 Result = FSavedMove_Character::GetCompressedFlags();

	if (Saved_bWantsToDash) Result |= FLAG_Dash;
	if (Saved_bPressedParkourJump) Result |= FLAG_JumpPressed;

	return Result;
}

#pragma endregion


#pragma region Client Network Prediction Data
UParkourMovementComponent::FNetworkPredictionData_Client_Parkour::FNetworkPredictionData_Client_Parkour(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UParkourMovementComponent::FNetworkPredictionData_Client_Parkour::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Parkour);
}

#pragma endregion


UParkourMovementComponent::UParkourMovementComponent()
{
	NavAgentProps.bCanCrouch = true;
}


#pragma region CMC

void UParkourMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();

	ParkourCharacterOwner = Cast<AParkourCharacter>(GetOwner());
}

// Network
void UParkourMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToDash = (Flags & FSavedMove_Parkour::FLAG_Dash) != 0;
}

FNetworkPredictionData_Client* UParkourMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (ClientPredictionData == nullptr)
	{
		UParkourMovementComponent* MutableThis = const_cast<UParkourMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Parkour(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

bool UParkourMovementComponent::IsMovingOnGround() const
{
	return Super::IsMovingOnGround() || IsCustomMovementMode(CMOVE_Slide);
}

bool UParkourMovementComponent::CanCrouchInCurrentState() const
{
	return Super::CanCrouchInCurrentState() && IsMovingOnGround();
}

float UParkourMovementComponent::GetMaxSpeed() const
{
	if (MovementMode != MOVE_Custom) return Super::GetMaxSpeed();

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return MaxSlideSpeed;
	case CMOVE_WallRun:
		return MaxWallRunSpeed;
	case CMOVE_Hang:
		return 0.f;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"));
		return -1.f;
	}
}

float UParkourMovementComponent::GetMaxBrakingDeceleration() const
{
	if (MovementMode != MOVE_Custom) return Super::GetMaxBrakingDeceleration();

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		return BreakingDecelerationSliding;
	case CMOVE_WallRun:
		return 0.f;
	case CMOVE_Hang:
		return 0.f;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
			return -1.f;
	}
}

bool UParkourMovementComponent::CanAttemptJump() const
{
	return Super::CanAttemptJump() || IsWallRunning();
}

bool UParkourMovementComponent::DoJump(bool bReplayingMoves)
{
	bool bWasWallRunning = IsWallRunning();

	FWallInfo* WallInfo = new FWallInfo();
	GetWallDetails(WallInfo);

	if (Super::DoJump(bReplayingMoves))
	{
		if (bWasWallRunning)
		{
			FVector Start = UpdatedComponent->GetComponentLocation();
			FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
			FVector End = Safe_bWallRunIsRight ? Start + CastDelta : Start - CastDelta;
			auto Params = ParkourCharacterOwner->GetIgnoreCharacterParams();
			FHitResult WallHit;
			GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
			Velocity += WallHit.Normal * WallJumpOffForce;
		}

		return true;
	}

	return false;
}

#pragma endregion


#pragma region MOVEMENT PIPELINE

void UParkourMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	// Slide
	if (MovementMode == MOVE_Walking && Safe_bWantsToSlide)
	{
		if (CanSlide())
		{
			SetMovementMode(MOVE_Custom, CMOVE_Slide);
		}
	}
	else if (IsCustomMovementMode(CMOVE_Slide) && bWantsToCrouch)
	{
		SetMovementMode(MOVE_Walking);
	}
	else if (IsCustomMovementMode(CMOVE_Slide) && ParkourCharacterOwner->bPressedJump)
	{
		SetMovementMode(MOVE_Falling);
	}

	// Dash
	bool bAuthProxy = CharacterOwner->HasAuthority() && !CharacterOwner->IsLocallyControlled();
	if (Safe_bWantsToDash && CanDash())
	{
		if (!bAuthProxy || GetWorld()->GetTimeSeconds() - DashStartTime > AuthDashCooldownDuration)
		{
			PerformDash();
			Safe_bWantsToDash = false;
			Proxy_bDash = !Proxy_bDash;
		}
		else
		{
			SLOG("Client Tried to cheat");
			UE_LOG(LogTemp, Warning, TEXT("Client tried to cheat"))
		}
	}

	// Wall Run
	if (IsFalling())
	{
		TryWallRun();
	}

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

}

void UParkourMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);

	if (!HasAnimRootMotion() && Safe_bHadAnimRootMotion && IsMovementMode(MOVE_Flying))
	{
		UE_LOG(LogTemp, Warning, TEXT("Ending Anim Root Motion"));
		SetMovementMode(MOVE_Walking);
	}

	if (GetRootMotionSourceByID(TransitionRMS_ID) && GetRootMotionSourceByID(TransitionRMS_ID)->Status.HasFlag(ERootMotionSourceStatusFlags::Finished))
	{
		RemoveRootMotionSourceByID(TransitionRMS_ID);
		Safe_bTransitionFinished = true;
	}

	Safe_bHadAnimRootMotion = HasAnimRootMotion();

}

void UParkourMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_Slide:
		PhysSlide(deltaTime, Iterations);
		break;
	case CMOVE_WallRun:
		PhysWallRun(deltaTime, Iterations);
		break;
	case CMOVE_Hang:
		break;
	default:
		UE_LOG(LogTemp, Fatal, TEXT("Invalid Movement Mode"))
	}
}

void UParkourMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	Safe_bWantsToSlide = bWantsToCrouch;
}

void UParkourMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Slide) ExitSlide();

	if (IsCustomMovementMode(CMOVE_Slide)) EnterSlide(PreviousMovementMode, (ECustomMovementMode)PreviousCustomMode);

	if (IsFalling())
	{
		bOrientRotationToMovement = true;
	}

	if (IsWallRunning() && GetOwnerRole() == ROLE_SimulatedProxy)
	{
		FVector Start = UpdatedComponent->GetComponentLocation();
		FVector End = Start + UpdatedComponent->GetRightVector() * CapR() * 2;
		auto Params = ParkourCharacterOwner->GetIgnoreCharacterParams();
		FHitResult WallHit;
		Safe_bWallRunIsRight = GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
	}

}

#pragma endregion


#pragma region SLIDE
void UParkourMovementComponent::EnterSlide(EMovementMode PrevMode, ECustomMovementMode PrevCustomMode)
{
	bWantsToCrouch = true;
	bOrientRotationToMovement = false;
	Velocity += Velocity.GetSafeNormal2D() * SlideEnterImpulse;

	FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, true, NULL);
}

void UParkourMovementComponent::ExitSlide()
{
	bWantsToCrouch = false;
	bOrientRotationToMovement = true;
}

bool UParkourMovementComponent::CanSlide() const
{
	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector End = Start + CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() * 2.5f * FVector::DownVector;
	FName ProfileName = TEXT("BlockAll");
	bool bValidSurface = GetWorld()->LineTraceTestByProfile(Start, End, ProfileName, ParkourCharacterOwner->GetIgnoreCharacterParams());
	bool bEnoughSpeed = Velocity.SizeSquared() > pow(MinSlideSpeed, 2);

	return bValidSurface && bEnoughSpeed;
}

void UParkourMovementComponent::PhysSlide(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CanSlide())
	{
		SetMovementMode(MOVE_Walking);
		StartNewPhysics(deltaTime, Iterations);
		return;
	}

	bJustTeleported = false;
	bool bCheckedFall = false;
	bool bTriedLedgeMove = false;
	float remainingTime = deltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// Save current values
		UPrimitiveComponent* const OldBase = GetMovementBase();
		const FVector PreviousBaseLocation = (OldBase != NULL) ? OldBase->GetComponentLocation() : FVector::ZeroVector;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FFindFloorResult OldFloor = CurrentFloor;

		// Ensure velocity is horizontal.
		MaintainHorizontalGroundVelocity();
		const FVector OldVelocity = Velocity;

		FVector SlopeForce = CurrentFloor.HitResult.Normal;
		SlopeForce.Z = 0.f;
		Velocity += SlopeForce * SlideGravityForce * deltaTime;

		Acceleration = Acceleration.ProjectOnTo(UpdatedComponent->GetRightVector().GetSafeNormal2D());

		// Apply acceleration
		CalcVelocity(timeTick, GroundFriction * SlideFrictionFactor, false, GetMaxBrakingDeceleration());

		// Compute move parameters
		const FVector MoveVelocity = Velocity;
		const FVector Delta = timeTick * MoveVelocity;
		const bool bZeroDelta = Delta.IsNearlyZero();
		FStepDownResult StepDownResult;
		bool bFloorWalkable = CurrentFloor.IsWalkableFloor();

		if (bZeroDelta)
		{
			remainingTime = 0.f;
		}
		else
		{
			// try to move forward
			MoveAlongFloor(MoveVelocity, timeTick, &StepDownResult);

			if (IsFalling())
			{
				// pawn decided to jump up
				const float DesiredDist = Delta.Size();
				if (DesiredDist > KINDA_SMALL_NUMBER)
				{
					const float ActualDist = (UpdatedComponent->GetComponentLocation() - OldLocation).Size2D();
					remainingTime += timeTick * (1.f - FMath::Min(1.f, ActualDist / DesiredDist));
				}
				StartNewPhysics(remainingTime, Iterations);
				return;
			}
			else if (IsSwimming()) //just entered water
			{
				StartSwimming(OldLocation, OldVelocity, timeTick, remainingTime, Iterations);
				return;
			}
		}

		// Update floor.
		// StepUp might have already done it for us.
		if (StepDownResult.bComputedFloor)
		{
			CurrentFloor = StepDownResult.FloorResult;
		}
		else
		{
			FindFloor(UpdatedComponent->GetComponentLocation(), CurrentFloor, bZeroDelta, NULL);
		}


		// check for ledges here
		const bool bCheckLedges = !CanWalkOffLedges();
		if (bCheckLedges && !CurrentFloor.IsWalkableFloor())
		{
			// calculate possible alternate movement
			const FVector GravDir = FVector(0.f, 0.f, -1.f);
			const FVector NewDelta = bTriedLedgeMove ? FVector::ZeroVector : GetLedgeMove(OldLocation, Delta, GravDir);
			if (!NewDelta.IsZero())
			{
				// first revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, false);

				// avoid repeated ledge moves if the first one fails
				bTriedLedgeMove = true;

				// Try new movement direction
				Velocity = NewDelta / timeTick;
				remainingTime += timeTick;
				continue;
			}
			else
			{
				// see if it is OK to jump
				// @todo collision : only thing that can be problem is that oldbase has world collision on
				bool bMustJump = bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;

				// revert this move
				RevertMove(OldLocation, OldBase, PreviousBaseLocation, OldFloor, true);
				remainingTime = 0.f;
				break;
			}
		}
		else
		{
			// Validate the floor check
			if (CurrentFloor.IsWalkableFloor())
			{
				if (ShouldCatchAir(OldFloor, CurrentFloor))
				{
					HandleWalkingOffLedge(OldFloor.HitResult.ImpactNormal, OldFloor.HitResult.Normal, OldLocation, timeTick);
					if (IsMovingOnGround())
					{
						// If still walking, then fall. If not, assume the user set a different mode they want to keep.
						StartFalling(Iterations, remainingTime, timeTick, Delta, OldLocation);
					}
					return;
				}

				AdjustFloorHeight();
				SetBase(CurrentFloor.HitResult.Component.Get(), CurrentFloor.HitResult.BoneName);
			}
			else if (CurrentFloor.HitResult.bStartPenetrating && remainingTime <= 0.f)
			{
				// The floor check failed because it started in penetration
				// We do not want to try to move downward because the downward sweep failed, rather we'd like to try to pop out of the floor.
				FHitResult Hit(CurrentFloor.HitResult);
				Hit.TraceEnd = Hit.TraceStart + FVector(0.f, 0.f, MAX_FLOOR_DIST);
				const FVector RequestedAdjustment = GetPenetrationAdjustment(Hit);
				ResolvePenetration(RequestedAdjustment, Hit, UpdatedComponent->GetComponentQuat());
				bForceNextFloorCheck = true;
			}

			// check if just entered water
			if (IsSwimming())
			{
				StartSwimming(OldLocation, Velocity, timeTick, remainingTime, Iterations);
				return;
			}

			// See if we need to start falling.
			if (!CurrentFloor.IsWalkableFloor() && !CurrentFloor.HitResult.bStartPenetrating)
			{
				const bool bMustJump = bJustTeleported || bZeroDelta || (OldBase == NULL || (!OldBase->IsQueryCollisionEnabled() && MovementBaseUtility::IsDynamicBase(OldBase)));
				if ((bMustJump || !bCheckedFall) && CheckFall(OldFloor, CurrentFloor.HitResult, Delta, OldLocation, remainingTime, timeTick, Iterations, bMustJump))
				{
					return;
				}
				bCheckedFall = true;
			}
		}

		// Allow overlap events and such to change physics state and velocity
		if (IsMovingOnGround() && bFloorWalkable)
		{
			// Make velocity reflect actual move
			if (!bJustTeleported && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && timeTick >= MIN_TICK_TIME)
			{
				// TODO-RootMotionSource: Allow this to happen during partial override Velocity, but only set allowed axes?
				Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
				MaintainHorizontalGroundVelocity();
			}
		}

		// If we didn't move at all this iteration then abort (since future iterations will also be stuck).
		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}
	}


	FHitResult Hit;
	FQuat NewRotation = FRotationMatrix::MakeFromXZ(Velocity.GetSafeNormal2D(), FVector::UpVector).ToQuat();
	SafeMoveUpdatedComponent(FVector::ZeroVector, NewRotation, false, Hit);
}

#pragma endregion


#pragma region DASH

void UParkourMovementComponent::OnDashCooldownFinished()
{
	Safe_bWantsToDash = true;
}

bool UParkourMovementComponent::CanDash() const
{
	return IsWalking() && !IsCrouching() || IsFalling();
}

void UParkourMovementComponent::PerformDash()
{
	DashStartTime = GetWorld()->GetTimeSeconds();

	SetMovementMode(MOVE_Flying);

	CharacterOwner->PlayAnimMontage(DashMontage);

	DashStartDelegate.Broadcast();
}

#pragma endregion


#pragma region WALL RUN
bool UParkourMovementComponent::TryWallRun()
{
	if (!IsFalling()) return false;
	if (Velocity.SizeSquared2D() < pow(MinWallRunSpeed, 2)) return false;
	if (Velocity.Z < -MaxVerticalWallRunSpeed) return false;

	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector LeftEnd = Start - UpdatedComponent->GetRightVector() * CapR() * 2;
	FVector RightEnd = Start + UpdatedComponent->GetRightVector() * CapR() * 2;

	auto Params = ParkourCharacterOwner->GetIgnoreCharacterParams();
	FHitResult FloorHit, WallHit;


	// Check Player Height
	if (GetWorld()->LineTraceSingleByProfile(FloorHit, Start, Start + FVector::DownVector * (CapHH() + MinWallRunHeight), "BlockAll", Params))
	{
		return false;
	}


	// Left Cast
	GetWorld()->LineTraceSingleByProfile(WallHit, Start, LeftEnd, "BlockAll", Params);
	if (WallHit.IsValidBlockingHit() && (Velocity | WallHit.Normal) < 0)
	{
		Safe_bWallRunIsRight = false;
	}

	// Right Cast
	else
	{
		GetWorld()->LineTraceSingleByProfile(WallHit, Start, RightEnd, "BlockAll", Params);
		if (WallHit.IsValidBlockingHit() && (Velocity | WallHit.Normal) < 0)
		{
			Safe_bWallRunIsRight = true;
		}
		else
		{
			return false;
		}
	}

	FVector ProjectedVelocity = FVector::VectorPlaneProject(Velocity, WallHit.Normal);
	if (ProjectedVelocity.SizeSquared2D() < pow(MinWallRunSpeed, 2)) return false;

	// Passed all conditions
	Velocity = ProjectedVelocity;
	Velocity.Z = FMath::Clamp(Velocity.Z, 0.f, MaxVerticalWallRunSpeed);
	SetMovementMode(MOVE_Custom, CMOVE_WallRun);
	SLOG("Starting WallRun!");
	return true;
}

void UParkourMovementComponent::PhysWallRun(float deltaTime, int32 Iterations)
{

	if (deltaTime < MIN_TICK_TIME)
	{
		return;
	}

	if (!CharacterOwner || (!CharacterOwner->Controller && !bRunPhysicsWithNoController && !HasAnimRootMotion() && !CurrentRootMotion.HasOverrideVelocity() && (CharacterOwner->GetLocalRole() != ROLE_SimulatedProxy)))
	{
		Acceleration = FVector::ZeroVector;
		Velocity = FVector::ZeroVector;
		return;
	}

	bJustTeleported = false;
	float remainingTime = deltaTime;

	// Perform the move
	while ((remainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations) && CharacterOwner && (CharacterOwner->Controller || bRunPhysicsWithNoController || (CharacterOwner->GetLocalRole() == ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported = false;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();

		FVector Start = UpdatedComponent->GetComponentLocation();
		FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
		FVector End = Safe_bWallRunIsRight ? Start + CastDelta : Start - CastDelta;

		auto Params = ParkourCharacterOwner->GetIgnoreCharacterParams();
		float SinPullAwayAngle = FMath::Sin(FMath::DegreesToRadians(WallRunPullAwayAngle));
		FHitResult WallHit;

		GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
		bool bWantsToPullAway = WallHit.IsValidBlockingHit() && !Acceleration.IsNearlyZero() && (Acceleration.GetSafeNormal() | WallHit.Normal) > SinPullAwayAngle;

		if (!WallHit.IsValidBlockingHit() || bWantsToPullAway)
		{
			SetMovementMode(MOVE_Falling);
			StartNewPhysics(remainingTime, Iterations);
			return;
		}

		// Clamp Acceleration
		Acceleration = FVector::VectorPlaneProject(Acceleration, WallHit.Normal);
		Acceleration.Z = 0.f;

		// Apply acceleration
		CalcVelocity(timeTick, 0.f, false, GetMaxBrakingDeceleration());
		Velocity = FVector::VectorPlaneProject(Velocity, WallHit.Normal);
		float TangentAccel = Acceleration.GetSafeNormal() | Velocity.GetSafeNormal2D();
		bool bVelUp = Velocity.Z > 0.f;

		if (WallRunGravityScaleCurve)
			Velocity.Z += GetGravityZ() * WallRunGravityScaleCurve->GetFloatValue(bVelUp ? 0.f : TangentAccel) * timeTick;
		else
			UE_LOG(LogTemp, Fatal, TEXT("You have not plugged in Gravity Scale Curve"));

		if (Velocity.SizeSquared2D() < pow(MinWallRunSpeed, 2) || Velocity.Z < -MaxVerticalWallRunSpeed)
		{
			SetMovementMode(MOVE_Falling);
			StartNewPhysics(remainingTime, Iterations);
			return;
		}

		// Compute move parameters
		const FVector Delta = timeTick * Velocity; // dx = v * dt
		const bool bZeroDelta = Delta.IsNearlyZero();
		if (bZeroDelta)
		{
			remainingTime = 0.f;
		}
		else
		{
			FHitResult Hit;
			SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
			FVector WallAttractionDelta = -WallHit.Normal * WallAttractionForce * timeTick;
			SafeMoveUpdatedComponent(WallAttractionDelta, UpdatedComponent->GetComponentQuat(), true, Hit);
		}

		if (UpdatedComponent->GetComponentLocation() == OldLocation)
		{
			remainingTime = 0.f;
			break;
		}

		Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick; // v = dx / dt
	}


	FVector Start = UpdatedComponent->GetComponentLocation();
	FVector CastDelta = UpdatedComponent->GetRightVector() * CapR() * 2;
	FVector End = Safe_bWallRunIsRight ? Start + CastDelta : Start - CastDelta;

	auto Params = ParkourCharacterOwner->GetIgnoreCharacterParams();
	FHitResult FloorHit, WallHit;

	GetWorld()->LineTraceSingleByProfile(WallHit, Start, End, "BlockAll", Params);
	GetWorld()->LineTraceSingleByProfile(FloorHit, Start, Start + FVector::DownVector * (CapHH() + MinWallRunHeight * .5f), "BlockAll", Params);

	if (FloorHit.IsValidBlockingHit() || !WallHit.IsValidBlockingHit() || Velocity.SizeSquared2D() < pow(MinWallRunSpeed, 2))
	{
		SetMovementMode(MOVE_Falling);
	}
}

#pragma endregion


#pragma region PARKOUR
bool UParkourMovementComponent::TryParkour(FWallInfo* WallInfo)
{

	return false;
}

#pragma endregion


#pragma region HELPERS
bool UParkourMovementComponent::IsServer() const
{
	return CharacterOwner->HasAuthority();
}

float UParkourMovementComponent::CapR() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float UParkourMovementComponent::CapHH() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

/// <summary>
///	Used for getting Wall related Information.
/// Debug Green indicate width and cyan indicates height.
/// </summary>
/// <param name="WallDetails"> - Fill the struct with info related to the wall detected.</param>
void UParkourMovementComponent::GetWallDetails(OUT FWallInfo* WallDetails)
{
	// Retrieve Basic Info
	FVector BaseLoc = UpdatedComponent->GetComponentLocation() + FVector::DownVector * CapHH();
	FVector Fwd = UpdatedComponent->GetForwardVector().GetSafeNormal2D();
	auto Params = ParkourCharacterOwner->GetIgnoreCharacterParams();
	FVector TopHit;

	// Raycasting front wall.
	FHitResult FrontHit;
	float checkDistance = FMath::Clamp(Velocity | Fwd, CapR() + 30, WallDistance);
	FVector	FrontStart = BaseLoc + FVector::UpVector * (MaxStepHeight - 1);

	for (int i = 0; i < 6; i++)
	{
		LINE(FrontStart, FrontStart + Fwd * checkDistance, FColor::Yellow);
		if (GetWorld()->LineTraceSingleByProfile(FrontHit, FrontStart, FrontStart + Fwd * checkDistance, "BlockAll", Params)) break;
		FrontStart += FVector::UpVector * (2.f * CapHH() - (MaxStepHeight - 1)) / 5;
	}

	// Checking if wall is there or not.
	if (!FrontHit.IsValidBlockingHit()) return;
	
	// Distance To Wall
	WallDetails->WallDistance = (FrontHit.Location - GetActorLocation()).Length();

	// Steepness of Wall : Angle 90deg is 1 and Angle 0deg is 0.
	WallDetails->wallAngleSteep = FrontHit.Normal | FVector::UpVector;
	
	// Collision Query Params.
	Params.bFindInitialOverlaps = false;
	Params.bTraceComplex = true;

	// Wall Vector and Angle
	FVector WallUp = FVector::VectorPlaneProject(FVector::UpVector, FrontHit.Normal).GetSafeNormal();
	float WallCos = FVector::UpVector | FrontHit.Normal;
	float WallSin = FMath::Sqrt(1 - WallCos * WallCos);


#pragma region ** Wall Height
	// Finding the closest wall from the top. Set Wall Height

	TArray<FHitResult> HeightHits;
	FHitResult SurfaceHit;
	FVector TraceStart = FrontHit.Location + Fwd + WallUp * (MaxWallHeight - (MaxStepHeight - 1)) / WallSin;

	POINT(FrontHit.Location, FColor::Magenta);
	POINT(TraceStart, FColor::Magenta);
	LINE(TraceStart, FrontHit.Location + Fwd, FColor::Purple);

	bool btraced = GetWorld()->LineTraceMultiByProfile(HeightHits, TraceStart, FrontHit.Location + Fwd, "BlockAll", Params);
	if (!btraced) return;
		
	for (const FHitResult& Hit : HeightHits)
	{
		if (Hit.IsValidBlockingHit())
		{
			SurfaceHit = Hit;
			TopHit = SurfaceHit.Location;
			WallDetails->wallSurfaceLocation = TopHit;
			POINT(TopHit, FColor::Magenta);
		}
	}

	if (SurfaceHit.IsValidBlockingHit())
	{
		WallDetails->wallHeight = (TopHit - BaseLoc) | FVector::UpVector;
	}
	else
	{
		return;
	}

#pragma endregion

#pragma region ** Wall Width
	FHitResult WidthHit;
	TArray<FHitResult> WidthHits;
	FVector WidthTraceStart = FrontHit.Location + Fwd + (FrontHit.Normal.GetSafeNormal2D() * MaxWallWidth * -1);

	LINE(FrontHit.Location, WidthTraceStart, FColor::Purple);
	POINT(WidthTraceStart, FColor::Green);
	
	btraced = GetWorld()->LineTraceMultiByProfile(WidthHits, WidthTraceStart, FrontHit.Location, "BlockAll", Params);
	if (!btraced) return;

	for (const FHitResult& Hit : WidthHits)
	{
		if (Hit.IsValidBlockingHit())
		{
			POINT(Hit.Location, FColor::Green);
			WidthHit = Hit;
			break;
		}
	}

	WallDetails->wallWidth = (WidthHit.Location - FrontHit.Location).Length();
	if (WallDetails->wallWidth > MaxWallWidth + 10.f)
	{
		WallDetails->wallWidth = -1.f;
	}

#pragma endregion

#pragma region ** Vault Location Details
	FVector EdgeVector = FrontHit.Normal ^ WallUp;
	FVector EdgeRight = TopHit + EdgeVector.GetSafeNormal2D() * ClearanceEdgeLen + Fwd;
	FVector EdgeLeft = TopHit - EdgeVector.GetSafeNormal2D() * ClearanceEdgeLen + Fwd;
	WallDetails->wallEdgeVector = EdgeVector;
	FHitResult EdgeWallHit;
	
	// Distance On Right
	if (GetWorld()->LineTraceSingleByProfile(EdgeWallHit, TopHit, EdgeRight, "BlockAll", Params))
	{
		WallDetails->ClearanceRightDistance = (EdgeWallHit.Location - TopHit).Length();
		ARROW(EdgeWallHit.Location, TopHit, 8.f, FColor::Red);
	}
	else
	{
		WallDetails->ClearanceRightDistance = ClearanceEdgeLen;
		ARROW(EdgeRight, TopHit, 8.f, FColor::Orange);
	}
	
	// Distance on Left
	if (GetWorld()->LineTraceSingleByProfile(EdgeWallHit, TopHit, EdgeLeft, "BlockAll", Params))
	{
		WallDetails->ClearanceLeftDistance = (EdgeWallHit.Location - TopHit).Length();
		ARROW(EdgeWallHit.Location, TopHit, 8.f, FColor::Red);
	}
	else
	{
		WallDetails->ClearanceLeftDistance = ClearanceEdgeLen;
		ARROW(EdgeLeft, TopHit, 8.f, FColor::Cyan);
	}
#pragma endregion

#ifdef DRAW_CLEARANCE
	float SurfaceCos = FVector::UpVector | SurfaceHit.Normal;
	float SurfaceSin = FMath::Sqrt(1 - SurfaceCos * SurfaceCos);

	FVector ClearCollisionLoc = TopHit + FrontHit.Normal.GetSafeNormal2D() * (ClearanceDepth + 1) * -1 + FVector::UpVector * (ClearanceHeight + 1);
	FQuat ClearCollisionRot = FRotationMatrix::MakeFromX(FrontHit.Normal.GetSafeNormal2D()).ToQuat();
	FVector ClearCollisionSize = FVector(ClearanceDepth, ClearanceWidth, ClearanceHeight);
	FCollisionShape BoxShape = FCollisionShape::MakeBox(ClearCollisionSize);

	if (GetWorld()->OverlapAnyTestByProfile(ClearCollisionLoc, ClearCollisionRot, "BlockAll", BoxShape, Params))
	{
		BOXROTATE(ClearCollisionLoc, ClearCollisionSize, ClearCollisionRot, FColor::Red);
		DLOG(FString::Printf(TEXT("Can't Fit Through!")), FColor::Red);
	}
	else
	{
		BOXROTATE(ClearCollisionLoc, ClearCollisionSize, ClearCollisionRot, FColor::Green);
	}
#endif


	if (bWallDebug)
	{	
		DLOG(FString::Printf(TEXT("Angle Steep: %f"), WallDetails->wallAngleSteep), FColor::Yellow);
		DLOG(FString::Printf(TEXT("Width: %f"), WallDetails->wallWidth), FColor::Yellow);
		DLOG(FString::Printf(TEXT("Height: %f"), WallDetails->wallHeight), FColor::Yellow);
		DLOG(FString::Printf(TEXT("Distance to wall: %f"), WallDetails->WallDistance), FColor::Yellow);
		DLOG(FString::Printf(TEXT("Wall Details")), FColor::Magenta);
		DLOG(FString::Printf(TEXT("**********---------**********")), FColor::Black);
	}
}

#pragma endregion


#pragma region INTERFACE
void UParkourMovementComponent::DashPressed()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - DashStartTime >= DashCooldownDuration)
	{
		Safe_bWantsToDash = true;
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_DashCooldown, this, &UParkourMovementComponent::OnDashCooldownFinished, DashCooldownDuration - (CurrentTime - DashStartTime));
	}
}

void UParkourMovementComponent::DashReleased()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_DashCooldown);
	Safe_bWantsToDash = false;
}

bool UParkourMovementComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

bool UParkourMovementComponent::IsMovementMode(EMovementMode InMovementMode) const
{
	return InMovementMode == MovementMode;
}

#pragma endregion


#pragma region REPLICATION

void UParkourMovementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UParkourMovementComponent, Proxy_bDash, COND_SkipOwner);
}

void UParkourMovementComponent::OnRep_Dash()
{
	CharacterOwner->PlayAnimMontage(DashMontage);
	DashStartDelegate.Broadcast();
}

#pragma endregion


