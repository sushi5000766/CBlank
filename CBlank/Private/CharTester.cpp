// Fill out your copyright notice in the Description page of Project Settings.


#include "CharTester.h"

// Sets default values
ACharTester::ACharTester()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ACharTester::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACharTester::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACharTester::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

