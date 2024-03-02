// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TestAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class CBLANK_API UTestAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UpdateAnimationProperties")
	void UpdateAnimationProperties(FVector2D Value);

protected:	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float updown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float rightleft;
};
