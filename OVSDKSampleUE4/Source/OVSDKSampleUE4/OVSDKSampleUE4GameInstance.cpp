// Fill out your copyright notice in the Description page of Project Settings.

#include "OVSDKSampleUE4.h"
#include "OVSDKSampleUE4GameInstance.h"
#include "OmniControllerPluginFunctionLibrary.h"
#include "OVInterface.h"
#include "OVStringRes.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/HeadMountedDisplayFunctionLibrary.h"

UOVSDKSampleUE4GameInstance::UOVSDKSampleUE4GameInstance()
	: Super()
{
}

void UOVSDKSampleUE4GameInstance::Init()
{
	UHeadMountedDisplayFunctionLibrary::EnableHMD(true);
#if USE_OVSDK_CPP
	UOVInterface::Init(1019, "01ec17dac7140c0fbe936ee128310000", "omni=1");
	UOVStringRes::LoadString("SampleLocalization");
#endif
}



