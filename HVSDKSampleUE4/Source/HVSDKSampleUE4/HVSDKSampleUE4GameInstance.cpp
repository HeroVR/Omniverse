// Fill out your copyright notice in the Description page of Project Settings.

#include "HVSDKSampleUE4.h"
#include "HVSDKSampleUE4GameInstance.h"
#include "OmniControllerPluginFunctionLibrary.h"
#include "HVInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"

UHVSDKSampleUE4GameInstance::UHVSDKSampleUE4GameInstance()
	: Super()
{
}

void UHVSDKSampleUE4GameInstance::Init()
{
	UHeadMountedDisplayFunctionLibrary::EnableHMD(true);
#if USE_HVSDK_CPP
	UHvInterface::init(1019, "01ec17dac7140c0fbe936ee128310000", "omni=1");
#endif
}



