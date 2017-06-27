// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "OVSDKSampleUE4.h"
#include "OVSDKSampleUE4GameMode.h"
#include "OVSDKSampleUE4GameInstance.h"
#include "OVSDKSampleUE4HUD.h"
#include "OVSDKSampleUE4Character.h"
#include "OVInterface.h"
#include "Engine.h"

AOVSDKSampleUE4GameMode::AOVSDKSampleUE4GameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AOVSDKSampleUE4HUD::StaticClass();
}

void AOVSDKSampleUE4GameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AOVSDKSampleUE4GameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

void AOVSDKSampleUE4GameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if USE_OVSDK_CPP
	if (UOVInterface::JustInitialized())
	{
		FString ret = UOVInterface::GetInitResultMsg();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, *ret);

		IPCUser *user = UOVInterface::GetUserInfo();
		char tmp[512];
		snprintf(tmp, 512, "user_id=%d\nuser_name=%s\nuser_phone=%s", user->nUserId, user->sUserName, user->sUserPhone);
		
		FString userinfo = tmp;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, *userinfo);
	}
#endif
}