// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePlayerUnit.h"
#include "TestPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
/**
 * 
 */
UCLASS()
class ATestPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ATestPlayerController();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SetMovementAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LeftClickAction;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ABasePlayerUnit> MyBlueprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Speed = 600.0f;  // Default speed

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RotationSpeedMin = 3.0f;  // Default speed

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RotationSpeedMax = 9.0f;  // Default speed

protected:
	//Functions
	virtual void BeginPlay();
	virtual void SetupInputComponent() override;
	virtual void Tick(float DeltaTime) override;

	void Move(const FInputActionValue& Value2);
	void MoveEnd();
	FVector PerformLineTraceFromCamera();
	FVector2D GetDirectonFromUnitToMouse(FVector MouseLoc);
	void UpdateActorRotation(FVector2d Direction);
	void LeftClick();
	void SetLocomotionValues();
	FVector2D GetFacingDirectionDifference(const FVector2D& MoveDirection);
	float ScaleInputBasedOnFacingDirection(const FVector2D& MoveDirection, float OriginalSpeed);

	//Variables
	APawn* PlayerPawn;
	FVector2D MoveDir;
};
