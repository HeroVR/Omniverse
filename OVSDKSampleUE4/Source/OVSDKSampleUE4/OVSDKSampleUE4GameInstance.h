// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "OVSDKSampleUE4GameInstance.generated.h"

#define USE_OVSDK_CPP 1


/**
 * 
 */
UCLASS()
class OVSDKSAMPLEUE4_API UOVSDKSampleUE4GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UOVSDKSampleUE4GameInstance();

	virtual void Init();
};
