// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "OVSDKSampleUE4.h"
#include "OVSDKSampleUE4Character.h"
#include "OVSDKSampleUE4GameInstance.h"
#include "OVSDKSampleUE4Projectile.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
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
	FirstPersonCameraComponent->bUsePawnControlRotation = false;
	FirstPersonCameraComponent->bLockToHmd = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->RelativeRotation = FRotator(1.9f, -19.19f, 5.2f);
	Mesh1P->RelativeLocation = FVector(-0.5f, -4.4f, -155.7f);

	// Create OmniControllerComponent
	OmniControllerComponent = CreateDefaultSubobject<UOmniControllerComponent>(TEXT("OmniControllerComponent"));
	OmniControllerComponent->InitOmniDone.AddDynamic(this, &AOVSDKSampleUE4Character::OnInitOmniDone);
	
	CorrectForwardSet = false;
}


void AOVSDKSampleUE4Character::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}


void AOVSDKSampleUE4Character::CorrectSpawnForward()
{
	APlayerController* theCont = static_cast<APlayerController*>(GetController());

	FRotator StartPointYaw(ForceInit);
	StartPointYaw.Yaw = theCont->StartSpot->GetActorRotation().Yaw;

	FRotator CameraYaw(ForceInit);
	CameraYaw.Yaw = FirstPersonCameraComponent->GetComponentRotation().Yaw;

	FRotator newRotation = StartPointYaw - CameraYaw;

	FRotator ControllerRotation(ForceInit);
	ControllerRotation.Yaw = theCont->GetControlRotation().Yaw;

	FRotator properOmniRotation = UKismetMathLibrary::ComposeRotators(newRotation, ControllerRotation);
	properOmniRotation.Pitch = 0.0f;
	properOmniRotation.Roll = 0.0f;

	theCont->SetControlRotation(properOmniRotation);
}


void AOVSDKSampleUE4Character::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!CorrectForwardSet)
	{
		if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
		{
			CorrectForwardSet = true;
			CorrectSpawnForward();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AOVSDKSampleUE4Character::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AOVSDKSampleUE4Character::OnResetVR);

	PlayerInputComponent->BindAxis("MoveForward", this, &AOVSDKSampleUE4Character::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AOVSDKSampleUE4Character::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &AOVSDKSampleUE4Character::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AOVSDKSampleUE4Character::TurnUp);
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
	UOmniControllerPluginFunctionLibrary::IsOmniFound(OmniFound);
	FRotator rotation(ForceInit);
	rotation.Yaw = FirstPersonCameraComponent->ComponentToWorld.GetRotation().Rotator().Yaw;
	FVector direction = rotation.Vector();


	if (OmniFound) 
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
	UOmniControllerPluginFunctionLibrary::IsOmniFound(OmniFound);
	FVector direction = FirstPersonCameraComponent->GetRightVector();

	if (OmniFound) 
	{
		Value *= OmniControllerComponent->GetStrafeSpeedMod();
		direction = FRotationMatrix(OmniControllerComponent->GetCurrentMovementDirection()).GetScaledAxis(EAxis::Y);
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
