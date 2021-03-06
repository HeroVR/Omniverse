// Fill out your copyright notice in the Description page of Project Settings.

#include "OmniversePrivatePCH.h"
#include "OmniControllerComponent.h"
#include "Runtime/Engine/Classes/Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values for this component's properties
UOmniControllerComponent::UOmniControllerComponent()
	:CouplingPercentage(1)
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bForceDeveloperMode = false;

	bAutoCorrectStartYaw = true;
	bStartYawCorrected = false;
	StartYawSet = false;
	StartYawDiff = 0;
	BackwardMovementMultiplier = 0.40f;
	StrafeSpeedMultiplier = 1.0f;
	AdjustedOmniYaw = 0.0f;

	MovementDirection = FRotator(0.0f, 0.0f, 0.0f);
}


// Called every frame
void UOmniControllerComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	APawn *pawn = Cast<APawn>(GetOwner());
	if (pawn)
	{
		AController *controller = pawn->GetController();
		if (controller && controller->IsLocalPlayerController())
		{
			// find Camera from Owner character;
			if (nullptr == Camera)
			{
				const TSet<UActorComponent*> &compos = pawn->GetComponents();
				for (UActorComponent *compo : compos)
				{
					if (compo 
						&& compo->IsA(UCameraComponent::StaticClass())
						&& (CameraTag == NAME_None || compo->ComponentHasTag(CameraTag)))
					{
						Camera = Cast<UCameraComponent>(compo);
						break;
					}
				}
			}

			CheckOmniState();

			if (!IsDeveloperMode())
			{
				// Raw yaw -> Game world yaw.
				UOmniControllerPluginFunctionLibrary::GetYaw(RawOmniYaw);
				CouplingPercentage = UOVInterface::GetOmniCoupleRate();

				if (RawOmniYaw > 180) {
					OmniYaw = RawOmniYaw - 360 - OmniYawOffset;
				}
				else {
					OmniYaw = RawOmniYaw - OmniYawOffset;
				}


				float cameraYaw = Camera ? Camera->GetComponentTransform().Rotator().Yaw : 0;

				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

				//Calculating the Angle Between the Camera and the Omni

				APlayerController *pc = CastChecked<APlayerController>(controller);
				float CharacterRotation = pawn->GetActorTransform().Rotator().Yaw;

				AdjustedOmniYaw = (RawOmniYaw - OmniYawOffset + CharacterRotation);
				float AngleBetweenCameraAndOmni = (int)FMath::Abs(cameraYaw - AdjustedOmniYaw) % 360;

				AngleBetweenCameraAndOmni = AngleBetweenCameraAndOmni > 180 ? 360 - AngleBetweenCameraAndOmni : AngleBetweenCameraAndOmni;

				float AngleForSignCalc = (int)(cameraYaw - AdjustedOmniYaw) % 360;

				float sign = (AngleForSignCalc >= 0 && AngleForSignCalc <= 180) ||
					(AngleForSignCalc <= -180 && AngleForSignCalc >= -360) ? 1 : -1;

				AngleBetweenCameraAndOmni *= sign;

				//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

				CurrYaw = (RawOmniYaw - OmniYawOffset + CharacterRotation) + (AngleBetweenCameraAndOmni * CouplingPercentage);

				//if (bAutoCorrectStartYaw && !StartYawSet)
				//{
				//	// Only want to set this rotation once. Causes motion sickness if allowed to set on Tick.
				//
				//	StartYawSet = true;
				//	float CameraForwardYaw = (RawOmniYaw - OmniYawOffset + CharacterRotation) + (AngleBetweenCameraAndOmni);
				//	float CameraOffsetFromCharacter = pawn->GetActorTransform().Rotator().Yaw - CameraForwardYaw;
				//
				//	FRotator CameraOffsetRotator = FRotator(ForceInit);
				//	CameraOffsetRotator.Yaw = CameraOffsetFromCharacter;
				//
				//	FRotator newRotation = FRotator(0.0f, 0.0f, 0.0f);
				//	newRotation = UKismetMathLibrary::ComposeRotators(CameraOffsetRotator, pc->GetControlRotation());
				//
				//	//pc->SetControlRotation(newRotation);
				//}

				MovementDirection = FRotator(0.0f, CurrYaw, 0.0f);
			}				
		}
	}

}


void UOmniControllerComponent::CheckOmniState()
{
	EOmniState::Type oldState = OmniState;
	OmniState = UOmniControllerPluginFunctionLibrary::GetState();
	if (oldState != OmniState)
	{
		if (oldState == EOmniState::Initing)
		{
			InitOmniDone.Broadcast();

			//UOmniControllerPluginFunctionLibrary::ResetStepCount();
		}

		OmniYawOffset = UOVInterface::GetOmniCalibrationOffset();
	}
}


bool UOmniControllerComponent::IsDeveloperMode() const
{
	if (bForceDeveloperMode) {
		return true;
	}

#if UE_BUILD_SHIPPING == 0
	return OmniState != EOmniState::Working;
#else
	return false;
#endif
}

void UOmniControllerComponent::AutoUpdateCharacterDirection()
{
	APawn *pawn = Cast<APawn>(GetOwner());
	if (pawn && Camera)
	{
		AController *controller = pawn->GetController();
		if (controller && controller->IsLocalPlayerController())
		{
			if (!IsDeveloperMode())
			{
				if (bAutoCorrectStartYaw && !bStartYawCorrected) {
					bStartYawCorrected = true;
					StartYawDiff = pawn->GetActorRotation().Yaw - OmniYaw;	//difference between Vive and Game
				}

				float cameraYaw = Camera ? Camera->RelativeRotation.Yaw : 0;
				float diff = FMath::Fmod(cameraYaw - OmniYaw, 360.0f);
				if (diff > 180.0f) {
					diff -= 360;
				}
				else if (diff < -180.0f) {
					diff += 360;
				}
				float characterYaw = StartYawDiff + OmniYaw + diff * CouplingPercentage ;

				// Set ACharacter Yaw
				//APlayerController *pc = CastChecked<APlayerController>(controller);
				//pc->SetControlRotation(FRotator(pc->RotationInput.Pitch, characterYaw, pc->RotationInput.Roll));
				FRotator charRot = pawn->GetActorRotation();
				pawn->SetActorRotation(FRotator(charRot.Pitch, characterYaw, charRot.Roll));

				if (Camera != nullptr) {
					Camera->GetAttachParent()->SetRelativeRotation(FQuat(FRotator(0, -pawn->GetActorRotation().Yaw + StartYawDiff, 0)));
				}
			}
		}
	}
}
