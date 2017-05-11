// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "HVSDKSampleUE4.h"
#include "HVSDKSampleUE4GameMode.h"
#include "HVSDKSampleUE4GameInstance.h"
#include "HVSDKSampleUE4HUD.h"
#include "HVSDKSampleUE4Character.h"
#include "HVInterface.h"
#include "Engine.h"

AHVSDKSampleUE4GameMode::AHVSDKSampleUE4GameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AHVSDKSampleUE4HUD::StaticClass();
}

void AHVSDKSampleUE4GameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AHVSDKSampleUE4GameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}

void AHVSDKSampleUE4GameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if USE_HVSDK_CPP
	if (UHvInterface::isJustInitDone())
	{
		FString ret = UHvInterface::getInitRet();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, *ret);

		IPCUser *user = UHvInterface::getUserInfo();
		char tmp[512];
		snprintf(tmp, 512, "user_id=%d\nuser_name=%s\nuser_phone=%s", user->nId, user->sName, user->sPhone);
		
		FString userinfo = tmp;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, *userinfo);
	}
#endif
}