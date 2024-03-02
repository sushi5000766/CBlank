// Fill out your copyright notice in the Description page of Project Settings.


#include "TestAnimInstance.h"

void UTestAnimInstance::UpdateAnimationProperties(FVector2D Value)
{
	//Get the pawn which is affected by our anim instance
	APawn* Pawn = TryGetPawnOwner();

	if (Pawn)
	{
		//Update our falling property
		updown = Value.X;

		//Update our movement speed
		rightleft = Value.Y;
	}
}