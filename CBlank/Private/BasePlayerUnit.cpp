// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayerUnit.h"
#include "Logging/LogMacros.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GASAbilitySystemComponent.h"
#include "GASAttributeSet.h"

// Sets default values
ABasePlayerUnit::ABasePlayerUnit()
{
	PrimaryActorTick.bCanEverTick = true;

	//ConstructorGAS start
	AbilitySystemComponent = CreateDefaultSubobject<UGASAbilitySystemComponent>("AbilitySystemComp");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	//ConstructorGAS end

	RootMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMeshComponent"));
	RootComponent = RootMeshComponent; // Set the root component

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->SetupAttachment(RootMeshComponent);
	CapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Overlap);

	// Create SkeletalMeshComponent
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(RootMeshComponent);
	SkeletalMeshComponent->SetRelativeLocation(FVector::ZeroVector);
	SkeletalMeshComponent->SetMobility(EComponentMobility::Movable);
	SkeletalMeshComponent->bEditableWhenInherited = true;

	// Create SpringArmComponent
	USpringArmComponent* SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootMeshComponent);
	SpringArmComponent->SetRelativeRotation(FRotator(-66.0f, 0.f, 0.f));
	SpringArmComponent->SetUsingAbsoluteRotation(true);
	SpringArmComponent->TargetArmLength = 1200.0f;
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->CameraLagSpeed = 10.0f;
	SpringArmComponent->bDoCollisionTest = false;

	// Create a camera and attach to our spring arm
	UCameraComponent* Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("ActualCamera"));
	Camera->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	Camera->bAutoActivate = true;

	//AutoPossessPlayer = EAutoReceiveInput::Player0;
}

//***GAS START***

// Called when the game starts or when spawned
void ABasePlayerUnit::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		AttributeSetVar = AbilitySystemComponent->GetSet<UGASAttributeSet>();

		const_cast<UGASAttributeSet*>(AttributeSetVar)->HealthChangeDelegate.AddDynamic(this, &ABasePlayerUnit::OnHealthChangedNative);
		const_cast<UGASAttributeSet*>(AttributeSetVar)->ManaChangeDelegate.AddDynamic(this, &ABasePlayerUnit::OnManaChangedNative);
		const_cast<UGASAttributeSet*>(AttributeSetVar)->BaseAttackDamageChangeDelegate.AddDynamic(this, &ABasePlayerUnit::OnBaseAttackDamageChangedNative);
		const_cast<UGASAttributeSet*>(AttributeSetVar)->SpeedMultiplierChangeDelegate.AddDynamic(this, &ABasePlayerUnit::OnSpeedMultiplierChangedNative);
	}
}

UAbilitySystemComponent* ABasePlayerUnit::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ABasePlayerUnit::OnHealthChangedNative(float Health, int32 StackCount)
{
	OnHealthChange(Health, StackCount);
}

void ABasePlayerUnit::OnManaChangedNative(float Mana, int32 StackCount)
{
	OnManaChange(Mana, StackCount);
}

void ABasePlayerUnit::OnBaseAttackDamageChangedNative(float BaseAttackChange, int32 StackCount)
{
	OnBaseAttackDamageChange(BaseAttackChange, StackCount);
}

void ABasePlayerUnit::OnSpeedMultiplierChangedNative(float SpeedMultiplier, int32 StackCount)
{
	OnSpeedMultiplierChange(SpeedMultiplier, StackCount);
}

void ABasePlayerUnit::InitializeAbility(TSubclassOf<UGameplayAbility> AbilityToGet, int32 AbilityLevel)
{
	if (HasAuthority() && AbilitySystemComponent) {
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityToGet, AbilityLevel, 0));
	}
}

void ABasePlayerUnit::InitializeAbilityMulti(TArray<TSubclassOf<UGameplayAbility>> AbilitiesToAcquire, int32 AbilityLevel)
{
	for (TSubclassOf<UGameplayAbility> AbilitItem : AbilitiesToAcquire)
	{
		InitializeAbility(AbilitItem, AbilityLevel);
	}
}

void ABasePlayerUnit::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitializeAbilityMulti(InitialAbilities, 0);
}

void ABasePlayerUnit::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void ABasePlayerUnit::GetHealthValues(float& Health, float& MaxHealth)
{
	if (AttributeSetVar)
	{
		Health = AttributeSetVar->GetHealth();
		MaxHealth = AttributeSetVar->GetMaxHealth();
	}
}

void ABasePlayerUnit::GetManaValues(float& Mana, float& MaxMana)
{
	if (AttributeSetVar)
	{
		Mana = AttributeSetVar->GetMana();
		MaxMana = AttributeSetVar->GetMaxMana();
	}
}

void ABasePlayerUnit::GetBaseAttackDamageValues(float& BaseAttackDamage)
{
	if (AttributeSetVar)
	{
		BaseAttackDamage = AttributeSetVar->GetBaseAttackDamage();
	}
}

void ABasePlayerUnit::GetSpeedMultiplierValues(float& SpeedMultiplier)
{
	if (AttributeSetVar)
	{
		SpeedMultiplier = AttributeSetVar->GetSpeedMultiplier();
	}
}

void ABasePlayerUnit::RemoveAbilityWithTags(FGameplayTagContainer TagContainer)
{
	TArray<FGameplayAbilitySpec*> MatchingAbilities;
	AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(TagContainer, MatchingAbilities, true);
	for (FGameplayAbilitySpec* Spec : MatchingAbilities)
	{
		AbilitySystemComponent->ClearAbility(Spec->Handle);
	}
}

void ABasePlayerUnit::ChangeAbilityLevelWithTags(FGameplayTagContainer TagContainer, int32 NewLevel)
{
	TArray<FGameplayAbilitySpec*> MatchingAbilities;
	AbilitySystemComponent->GetActivatableGameplayAbilitySpecsByAllMatchingTags(TagContainer, MatchingAbilities, true);
	for (FGameplayAbilitySpec* Spec : MatchingAbilities)
	{
		Spec->Level = NewLevel;
	}
}

void ABasePlayerUnit::CancelAbilityWithTags(FGameplayTagContainer WithTags, FGameplayTagContainer WithoutTags)
{
	AbilitySystemComponent->CancelAbilities(&WithTags, &WithoutTags, nullptr);
}

void ABasePlayerUnit::AddLooseGameplayTag(FGameplayTag TagToAdd)
{
	GetAbilitySystemComponent()->AddLooseGameplayTag(TagToAdd);
	GetAbilitySystemComponent()->SetTagMapCount(TagToAdd, 1);
}

void ABasePlayerUnit::RemoveLooseGameplayTags(FGameplayTag TagToRemove)
{
	GetAbilitySystemComponent()->RemoveLooseGameplayTag(TagToRemove);
}

//***GAS END***