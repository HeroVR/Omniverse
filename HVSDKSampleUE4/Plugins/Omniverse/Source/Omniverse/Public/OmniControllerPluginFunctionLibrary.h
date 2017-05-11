// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "OmniControllerPluginFunctionLibrary.generated.h"

UENUM(BlueprintType)
namespace EOmniState
{
	enum Type {
		Initing,
		NoOmni,
		Working,
	};
}

UCLASS()
class OMNIVERSE_API UOmniControllerPluginFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Init Omni", Keywords = "init omni"), Category = "VirtuixOmni")
		static void InitOmni();

		UFUNCTION(BlueprintCallable, meta = (DisplayName = "Reset Step Count", Keywords = "reset step omni"), Category = "VirtuixOmni")
		static void ResetStepCount();

		UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Step Count", Keywords = "get step omni"), Category = "VirtuixOmni")
		static void GetStepCount(int& Steps);

		UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Omni X-Axis From Plugin", Keywords = "get x axis omni"), Category = "VirtuixOmni")
		static void GetXAxis(float& XAxis);

		UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Omni Y-Axis From Plugin", Keywords = "get y axis omni"), Category = "VirtuixOmni")
		static void GetYAxis(float& YAxis);

		UFUNCTION(BlueprintPure, meta = (DisplayName = "Get Omni Yaw From Plugin", Keywords = "get yaw omni"), Category = "VirtuixOmni")
		static void GetYaw(float& Yaw);

		UFUNCTION(BlueprintPure, meta = (DisplayName = "Is Omni Found", Keywords = "is omni found"), Category = "VirtuixOmni")
		static void IsOmniFound(bool& Found);

		UFUNCTION(BlueprintPure, Category = "VirtuixOmni")
		static EOmniState::Type GetState();
};

