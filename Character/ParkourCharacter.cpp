#include "ParkourCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Parkour/Component/ParkourMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"


// Helper Macros
#if 1
float WriteDuration = 2.f;
#define SLOG(c, x) GEngine->AddOnScreenDebugMessage(-1, WriteDuration ? WriteDuration : -1.f, c, FString::Printf(TEXT(x)));
#else
#define SLOG(x, y);
#endif


AParkourCharacter::AParkourCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UParkourMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	ParkourMovement = Cast<UParkourMovementComponent>(GetCharacterMovement());

	bUseControllerRotationPitch = bUseControllerRotationYaw = bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Follow Camera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false;
}

void AParkourCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void AParkourCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AParkourCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Binding Axis
	PlayerInputComponent->BindAxis("Move Forward", this, &AParkourCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right", this, &AParkourCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AParkourCharacter::Turn);
	PlayerInputComponent->BindAxis("Look Up", this, &AParkourCharacter::LookUp);

	// Bind Action
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AParkourCharacter::ParkourJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AParkourCharacter::ParkourCrouch);
	PlayerInputComponent->BindAction("Slide", IE_Pressed, this, &AParkourCharacter::Slide);
	PlayerInputComponent->BindAction("Dash", IE_Pressed, this, &AParkourCharacter::DashPressed);
	PlayerInputComponent->BindAction("Dash", IE_Released, this, &AParkourCharacter::DashReleased);
}

#pragma region INPUT

void AParkourCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) && !GetCharacterMovement()->IsFalling())
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AParkourCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f) && !GetCharacterMovement()->IsFalling())
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AParkourCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AParkourCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AParkourCharacter::ParkourJump()
{
	Super::Jump();
}

void AParkourCharacter::ParkourCrouch()
{
	if (bIsCrouched)
	{
		Super::Crouch();
	}
	else
	{
		Super::UnCrouch();
	}
}

void AParkourCharacter::Slide()
{
	SLOG(FColor::Yellow, "[Warning]: NO LOGIC FOR SLIDE IMPLEMENTED YET!")
}

void AParkourCharacter::DashPressed() 
{
	ParkourMovement->DashPressed();
}

void AParkourCharacter::DashReleased()
{
	ParkourMovement->DashReleased();
}

#pragma endregion

#pragma region Helpers

FCollisionQueryParams AParkourCharacter::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams Params;

	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}

FVector AParkourCharacter::GetUnitVelocityDirection() const
{
	FVector vel = GetVelocity();
	return vel.GetSafeNormal2D();
}

#pragma endregion
