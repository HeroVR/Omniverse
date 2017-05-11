// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "HVSDKSampleUE4GameInstance.generated.h"

#define USE_HVSDK_CPP 1


/**
 * 
 */
UCLASS()
class HVSDKSAMPLEUE4_API UHVSDKSampleUE4GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UHVSDKSampleUE4GameInstance();

	virtual void Init();
};
