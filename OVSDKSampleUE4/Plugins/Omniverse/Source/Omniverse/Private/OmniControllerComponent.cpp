// Fill out your copyright notice in the Description page of Project Settings.

#include "OmniversePrivatePCH.h"
#include "OmniControllerComponent.h"
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
	StartYawDiff = 0;
	BackwardMovementMod = 0.75f;
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
						&& (CameraTag != NAME_None || compo->ComponentHasTag(CameraTag)))
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

				//UOVInterface::GetOmniCalibrationOffset();

				if (RawOmniYaw > 180) {
					OmniYaw = RawOmniYaw - 360 - OmniYawOffset;
				}
				else {
					OmniYaw = RawOmniYaw - OmniYawOffset;
				}

				if (bAutoCorrectStartYaw && !bStartYawCorrected) {
					bStartYawCorrected = true;
					StartYawDiff = pawn->GetActorRotation().Yaw - OmniYaw;	//difference between Vive and Game
				}

				float cameraYaw = Camera ? Camera->GetComponentTransform().Rotator().Yaw : 0;
				CurrYaw = cameraYaw * CouplingPercentage + (OmniYaw + StartYawDiff) * (1.0f - CouplingPercentage);

				// Set ACharacter Yaw
				APlayerController *pc = CastChecked<APlayerController>(controller);
				pc->SetControlRotation(FRotator(pc->RotationInput.Pitch, CurrYaw, pc->RotationInput.Roll));

				Camera->GetAttachParent()->SetRelativeRotation(FQuat(FRotator(0, -pawn->GetActorRotation().Yaw + StartYawDiff, 0)));
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
