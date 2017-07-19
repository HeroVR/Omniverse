// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "OVSDKSampleUE4.h"
#include "OVSDKSampleUE4Character.h"
#include "OVSDKSampleUE4GameInstance.h"
#include "OVSDKSampleUE4Projectile.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "MotionControllerComponent.h"
#include "OVInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AOVSDKSampleUE4Character

AOVSDKSampleUE4Character::AOVSDKSampleUE4Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	VRCameraBase = CreateDefaultSubobject<USceneComponent>(TEXT("VRCameraBase"));
	VRCameraBase->SetupAttachment(GetCapsuleComponent());

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(VRCameraBase);
	FirstPersonCameraComponent->RelativeLocation = FVector(-39.56f, 1.75f, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bLockToHmd = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(true);			// only the owning player will see this mesh
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun);
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.2f, 48.4f, -10.6f));

	// Default offset from the character location for projectiles to spawn
	GunOffset = FVector(100.0f, 30.0f, 10.0f);

	// Create OmniControllerComponent
	OmniControllerComponent = CreateDefaultSubobject<UOmniControllerComponent>(TEXT("OmniControllerComponent"));
	OmniControllerComponent->InitOmniDone.AddDynamic(this, &AOVSDKSampleUE4Character::OnInitOmniDone);
	//OmniControllerComponent->Camera = FirstPersonCameraComponent;
}

void AOVSDKSampleUE4Character::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
}

void AOVSDKSampleUE4Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FirstPersonCameraComponent->bUsePawnControlRotation = !UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled();
}

//////////////////////////////////////////////////////////////////////////
// Input

void AOVSDKSampleUE4Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AOVSDKSampleUE4Character::OnFire);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AOVSDKSampleUE4Character::OnResetVR);

	PlayerInputComponent->BindAxis("MoveForward", this, &AOVSDKSampleUE4Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AOVSDKSampleUE4Character::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AOVSDKSampleUE4Character::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AOVSDKSampleUE4Character::TurnUp);
}


void AOVSDKSampleUE4Character::OnFire()
{
	// try and fire a projectile
	if (ProjectileClass != NULL)
	{
		UWorld* const World = GetWorld();
		if (World != NULL)
		{
			{
				FRotator SpawnRotation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentRotation() : GetControlRotation());
				// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
				const FVector SpawnLocation = ((FP_MuzzleLocation != nullptr) ? FP_MuzzleLocation->GetComponentLocation() : GetActorLocation()) + SpawnRotation.RotateVector(GunOffset);

				// spawn the projectile at the muzzle
				World->SpawnActor<AOVSDKSampleUE4Projectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			}
		}
	}

	// try and play the sound if specified
	if (FireSound != NULL)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != NULL)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != NULL)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

void AOVSDKSampleUE4Character::OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AOVSDKSampleUE4Character::MoveForward(float Value)
{
	if (Value == 0.0f)
		return;
	
	bool OmniFound = false;
	FRotator rotation(ForceInit);
	rotation.Yaw = FirstPersonCameraComponent->ComponentToWorld.GetRotation().Rotator().Yaw;
	FVector direction = rotation.Vector();
	
	UOmniControllerPluginFunctionLibrary::IsOmniFound(OmniFound);
	
	if(OmniFound)
	{
		if(Value < 0)
			Value *= OmniControllerComponent->GetBackwardMovementMod();
		
		direction = FRotationMatrix(OmniControllerComponent->GetCurrentMovementDirection()).GetScaledAxis(EAxis::X);
	}
	AddMovementInput(direction, Value);			
}

void AOVSDKSampleUE4Character::MoveRight(float Value)
{
	if (Value == 0.0f)
		return;

	bool OmniFound = false;
	FVector direction = FirstPersonCameraComponent->GetRightVector();

	UOmniControllerPluginFunctionLibrary::IsOmniFound(OmniFound);

	if (OmniFound)
	{
		direction = FRotationMatrix(OmniControllerComponent->GetCurrentMovementDirection()).GetScaledAxis(EAxis::Y);

		Value *= OmniControllerComponent->GetStrafeSpeedMod();
	}

	AddMovementInput(direction, Value);
}

void AOVSDKSampleUE4Character::Turn(float Rate)
{
	if (OmniControllerComponent->IsDeveloperMode()) {
		Super::AddControllerYawInput(Rate);
	}	
}

void AOVSDKSampleUE4Character::TurnUp(float Rate)
{
	if (!UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled()) {
		Super::AddControllerPitchInput(Rate);
	}
}

void AOVSDKSampleUE4Character::OnInitOmniDone()
{
	static bool hasInited = false;
	if (!hasInited) {
		hasInited = true;
		UOmniControllerPluginFunctionLibrary::ResetStepCount();
	}
}
