// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
//
#include "OmniversePrivatePCH.h"
#include "OmniControllerPluginFunctionLibrary.h"
#include "OmniInputDevice.h"

UOmniControllerPluginFunctionLibrary::UOmniControllerPluginFunctionLibrary(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}

void UOmniControllerPluginFunctionLibrary::InitOmni()
{
	if (FOmniverseModule::Get().OmniInputDevice.IsValid())
	{
		FOmniverseModule::Get().OmniInputDevice->Init();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Omni not initialized. Try adding a delay before calling if calling at startup."));
	}

}

void UOmniControllerPluginFunctionLibrary::ResetStepCount()
{
	if (FOmniverseModule::Get().OmniInputDevice.IsValid())
	{
		FOmniverseModule::Get().OmniInputDevice->bResetStepCount = true;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Omni not initialized to Reset Setup Count. Try adding a delay before calling if calling at startup."));
	}
}

void UOmniControllerPluginFunctionLibrary::GetStepCount(int& Steps)
{
	int currentSteps = FOmniverseModule::Get().OmniInputDevice->iCurrentStepCount;
	if (currentSteps < 0) currentSteps = 0;
	Steps = currentSteps;
}

void UOmniControllerPluginFunctionLibrary::GetXAxis(float& XAxis)
{
	XAxis = FOmniverseModule::Get().OmniInputDevice->XAxis;
}

void UOmniControllerPluginFunctionLibrary::GetYAxis(float& YAxis)
{
	YAxis = FOmniverseModule::Get().OmniInputDevice->YAxis;
}

void UOmniControllerPluginFunctionLibrary::GetYaw(float& Yaw)
{
	Yaw = FOmniverseModule::Get().OmniInputDevice->OmniYaw;
}

void UOmniControllerPluginFunctionLibrary::IsOmniFound(bool& Found)
{
	Found = FOmniverseModule::Get().OmniInputDevice->bInitializationSucceeded;
}

EOmniState::Type UOmniControllerPluginFunctionLibrary::GetState()
{
	EOmniState::Type ret = EOmniState::Initing;

	TSharedPtr<class FOmniInputDevice> &device = FOmniverseModule::Get().OmniInputDevice;
	if (device.IsValid()) {
		ret = device->bInitializationSucceeded ? EOmniState::Working : EOmniState::NoOmni;
	}

	return ret;
}