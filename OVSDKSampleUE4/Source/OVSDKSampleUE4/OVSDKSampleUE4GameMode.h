// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameMode.h"
#include "OVSDKSampleUE4GameMode.generated.h"

UCLASS(minimalapi)
class AOVSDKSampleUE4GameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AOVSDKSampleUE4GameMode();

	virtual void BeginPlay();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
	virtual void Tick(float DeltaSeconds);
};



