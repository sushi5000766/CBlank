// Fill out your copyright notice in the Description page of Project Settings.


#include "TestPlayerController.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include <Kismet/GameplayStatics.h>
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Logging/LogMacros.h"
#include <UObject/UnrealTypePrivate.h>
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "TestAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"

ATestPlayerController::ATestPlayerController() 
{
	bShowMouseCursor = true;
}

void ATestPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		Subsystem->AddMappingContext(DefaultMappingContext, 0);
	}	

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // Ignore collisions

	UnPossess();
	FVector SpawnLoc(0.0f, 0.0f, 0.0f);
	ABasePlayerUnit* testpawn = GetWorld()->SpawnActor<ABasePlayerUnit>(MyBlueprint, FTransform(SpawnLoc), SpawnParams);
	Possess(testpawn);

	PlayerPawn = ATestPlayerController::GetPawn();
	MoveDir = FVector2D(0.0f, 0.0f);
}

void ATestPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(SetMovementAction, ETriggerEvent::Triggered, this, &ATestPlayerController::Move);
		EnhancedInputComponent->BindAction(SetMovementAction, ETriggerEvent::Canceled, this, &ATestPlayerController::MoveEnd);
		EnhancedInputComponent->BindAction(SetMovementAction, ETriggerEvent::Completed, this, &ATestPlayerController::MoveEnd);
		EnhancedInputComponent->BindAction(LeftClickAction, ETriggerEvent::Started, this, &ATestPlayerController::LeftClick);
		
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}

	UE_LOG(LogTemp, Warning, TEXT("SetupInput function is ran success"));
}

void ATestPlayerController::Move(const FInputActionValue& Value2) 
{
	FVector2D MoveValue = Value2.Get<FVector2D>();
	//UE_LOG(LogTemp, Warning, TEXT("Vector2D X: %f, Y: %f"), MoveValue.X, MoveValue.Y);
	FVector MoveVec = FVector(MoveValue.X, MoveValue.Y, 0);

	float SpeedMod = ScaleInputBasedOnFacingDirection(MoveDir, Speed);

	if (PlayerPawn != nullptr)
	{
		FVector NormDir = MoveVec.GetSafeNormal();
		FVector NewLocation = PlayerPawn->GetActorLocation() + NormDir * SpeedMod * GetWorld()->GetDeltaSeconds();
		PlayerPawn->SetActorLocation(NewLocation);		
		MoveDir = GetFacingDirectionDifference(MoveValue);
	}
}

void ATestPlayerController::MoveEnd()
{
	MoveDir = FVector2D(0.0f, 0.0f);
}


FVector2D ATestPlayerController::GetFacingDirectionDifference(const FVector2D& MoveDirection)
{
	// Ensure that the move direction is normalized
	FVector2D NormalizedMoveDirection = MoveDirection.GetSafeNormal();

	// Get the forward vector of the pawn
	FVector ForwardVector = PlayerPawn->GetActorForwardVector();

	// Calculate the dot product between the forward vector and move direction
	float DotProductX = FVector::DotProduct(FVector(ForwardVector.X, ForwardVector.Y, 0.0f), FVector(NormalizedMoveDirection.X, NormalizedMoveDirection.Y, 0.0f));
	float DotProductY = FVector::DotProduct(FVector(ForwardVector.X, ForwardVector.Y, 0.0f), FVector(NormalizedMoveDirection.Y, -NormalizedMoveDirection.X, 0.0f));

	// Calculate the angle between the vectors (in degrees)
	float AngleX = FMath::Acos(DotProductX) * (180.0f / PI);
	float AngleY = FMath::Acos(DotProductY) * (180.0f / PI);

	// Convert the angle difference to a value between -100 and 100
	float ResultX = FMath::Clamp((1.0f - AngleX / 180.0f) * 200.0f - 100.0f, -100.0f, 100.0f);
	float ResultY = FMath::Clamp((1.0f - AngleY / 180.0f) * 200.0f - 100.0f, -100.0f, 100.0f);

	// Print the output to the log
	//UE_LOG(LogTemp, Warning, TEXT("Facing Direction Difference - X: %f, Y: %f"), ResultX, ResultY);

	return FVector2D(ResultX, ResultY);
}

float ATestPlayerController::ScaleInputBasedOnFacingDirection(const FVector2D& InputVector, float OriginalSpeed)
{

	const float ForwardReduction = 0.0f; // No reduction when moving forward
	const float SideReduction = 0.10f;    // 15% reduction when moving left or right
	const float BackwardReduction = 0.25f; // 30% reduction when moving backward
	const float DiagonalReduction = 0.075f; // 7.5% reduction for diagonal movement

	float speeder = OriginalSpeed;
	float XReduction = 0.0f;
	float YReduction = 0.0f;

	// Reduction for X-axis (backward movement)
	if (InputVector.X < 0)
	{
		XReduction = BackwardReduction * FMath::Abs(InputVector.X / -100.0f);
	}

	// Reduction for Y-axis (side movement)
	if (FMath::Abs(InputVector.Y) > 0)
	{
		YReduction = SideReduction * FMath::Abs(InputVector.Y / 100.0f);
	}

	// Combine reductions for diagonal movement
	float TotalReduction = FMath::Min(XReduction + YReduction, 1.0f);

	// Calculate and return the modified speed
	float ModifiedSpeed = speeder * (1.0f - TotalReduction);

	// Print the output to the log
	//UE_LOG(LogTemp, Warning, TEXT("Original Speed: %f"), OriginalSpeed);
	//UE_LOG(LogTemp, Warning, TEXT("Final Speed: %f"), ModifiedSpeed);

	return ModifiedSpeed;
}

void ATestPlayerController::LeftClick()
{
	UE_LOG(LogTemp, Warning, TEXT("LeftClick"));
}

FVector ATestPlayerController::PerformLineTraceFromCamera()
{
	if (GetWorld())
	{
		// Get the player's viewport size
		FVector2D ViewportSize;
		int32 ViewportSizeX, ViewportSizeY; // Integer variables to store the viewport size

		GetViewportSize(ViewportSizeX, ViewportSizeY);

		// Get the mouse position
		FVector2D MousePosition;
		if (UGameplayStatics::GetPlayerController(GetWorld(), 0)->GetMousePosition(MousePosition.X, MousePosition.Y))
		{
			// Convert the 2D mouse position to a 3D world position
			FVector WorldLocation, WorldDirection;
			DeprojectScreenPositionToWorld(MousePosition.X, MousePosition.Y, WorldLocation, WorldDirection);

			// Set the end location for the line trace
			FVector LineTraceEnd = WorldLocation + (WorldDirection * 2000.0f);  // Adjust the distance as needed

			// Perform the line trace
			FHitResult HitResult;
			FCollisionQueryParams CollisionParams;
			CollisionParams.AddIgnoredActor(this);  // Ignore the actor performing the trace

			if (GetWorld()->LineTraceSingleByChannel(HitResult, WorldLocation, LineTraceEnd, ECC_Visibility, CollisionParams))
			{
				FVector HitLocation = HitResult.Location;
				//UE_LOG(LogTemp, Warning, TEXT("Hit Location: X=%f, Y=%f, Z=%f"), HitLocation.X, HitLocation.Y, HitLocation.Z);
				//DrawDebugLine(GetWorld(), WorldLocation, LineTraceEnd, FColor::Green, false, 2.0f, 0, 1.0f);
				return HitLocation;
			}		
			else
			{
				return FVector(0, 0, 0);
			}
		}
		else
		{
			return FVector(0, 0, 0);
		}
	}
	else
	{
		return FVector(0, 0, 0);
	}
}

FVector2D ATestPlayerController::GetDirectonFromUnitToMouse(FVector MouseLoc)
{	
	if (PlayerPawn != nullptr) {
		FVector PawnLoc = PlayerPawn->GetActorLocation();
		FVector2D DirectionToMouse = FVector2D(MouseLoc.X, MouseLoc.Y) - FVector2D(PawnLoc.X, PawnLoc.Y);
		return DirectionToMouse;
	}	
	else
	{
		return FVector2D(0, 0);
	}
}

void ATestPlayerController::UpdateActorRotation(FVector2d Direction)
{
	FRotator CurrentRotation = PlayerPawn->GetActorRotation();
	FRotator NewRotation = FRotationMatrix::MakeFromX(FVector(Direction.X, Direction.Y, 0.0f)).Rotator();
	
	FVector PlayerDist = PlayerPawn->GetActorLocation();
	FVector MouseDist = PerformLineTraceFromCamera();
	float Dist = FVector::Dist(FVector(PlayerDist.X, PlayerDist.Y, 0), FVector(MouseDist.X, MouseDist.Y, 0));
	float SpeedScale = FMath::GetMappedRangeValueClamped(FVector2D(50.0f, 500.0f), FVector2D(RotationSpeedMin, RotationSpeedMax), Dist);
	
	FRotator SmoothRotation = FMath::RInterpTo(CurrentRotation, NewRotation, GetWorld()->GetDeltaSeconds(), SpeedScale);
	if (PlayerPawn != nullptr) {

		PlayerPawn->SetActorRotation(SmoothRotation);
		//UE_LOG(LogTemp, Warning, TEXT("RotationUpdate"));
	}
}

void ATestPlayerController::SetLocomotionValues()
{
	if (PlayerPawn != nullptr)
	{
		USkeletalMeshComponent* SkeletalMeshComp = PlayerPawn->FindComponentByClass<USkeletalMeshComponent>();
		if (SkeletalMeshComp)
		{
			UAnimInstance* AnimInstance = SkeletalMeshComp->GetAnimInstance();

			if (AnimInstance)
			{
				// Cast to your specific animation instance class
				UTestAnimInstance* YourAnimInstance = Cast<UTestAnimInstance>(AnimInstance);

				if (YourAnimInstance)
				{
					// Set the float values in the Animation Instance
					YourAnimInstance->UpdateAnimationProperties(MoveDir);
					//UE_LOG(LogTemp, Warning, TEXT("Locomotion Update"));
				}
			}
		}
	}
}

void ATestPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector MouseLoc = PerformLineTraceFromCamera();
	UpdateActorRotation(GetDirectonFromUnitToMouse(MouseLoc));
	//UE_LOG(LogTemp, Warning, TEXT("PerformLineTraceFromCamera function is running"));
	//UE_LOG(LogTemp, Warning, TEXT("Facing Direction Difference - X: %f, Y: %f"), MoveDir.X, MoveDir.Y);
	SetLocomotionValues();
}