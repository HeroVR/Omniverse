// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"
#include "IInputDeviceModule.h"
#include "Runtime/Slate/Public/Framework/Application/SlateApplication.h"
#include "WindowsApplication.h"
//#include "OmniInputDevice.h"


class FWindowsHandler
	: public IWindowsMessageHandler
{
public:

	//~ IWindowsMessageHandler interface

	virtual bool ProcessMessage(HWND Hwnd, uint32 Message, WPARAM WParam, LPARAM LParam, int32& OutResult) override;

};


class FOmniInputDevice;

class FOmniverseModule : public IInputDeviceModule
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void PostLoadCallback() override;
	virtual void PreUnloadCallback() override;
	virtual void ShutdownModule() override;

	virtual TSharedPtr< class IInputDevice > FOmniverseModule::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override;

	TSharedPtr< class FOmniInputDevice > OmniInputDevice;

	static inline FOmniverseModule& Get()
	{
		//NOTE: The name must match the module name from the ".uplugin" file under the "Modules" section. 
		return FModuleManager::LoadModuleChecked< FOmniverseModule >("Omniverse");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("Omniverse");
	}

	FWindowsApplication* GetApplication() const
	{
		if (!FSlateApplication::IsInitialized())
			return nullptr;

		return (FWindowsApplication*)FSlateApplication::Get().GetPlatformApplication().Get();
	}

private:

	FWindowsHandler Handler;

};
