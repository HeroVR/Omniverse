// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "OmniversePrivatePCH.h"
#include "OmniInputDevice.h"
#include "SlateBasics.h"
#include "Runtime/Core/Public/Features/IModularFeatures.h"
#include "Runtime/InputDevice/Public/IInputDeviceModule.h"
#include "WIndows/WIndowsPlatformProcess.h"
// #include "hidapi.h"

#define LOCTEXT_NAMESPACE "FOmniverseModule"
#define DeviceArrival 0x8000
#define DeviceChanged 0x0007



#if PLATFORM_WINDOWS
#	if PLATFORM_64BITS
#		define DESKTOP_PLUGIN_DLL	TEXT("Omniverse/DLL/Win64/desktop_plugin.dll")
#	else
#		define DESKTOP_PLUGIN_DLL	TEXT("Omniverse/DLL/Win32/desktop_plugin.dll")
#	endif
#else
#      error Unsupported platform for HVSDK
#endif

void FOmniverseModule::StartupModule()
{
	IInputDeviceModule::StartupModule();

	UE_LOG(LogTemp, Display, TEXT("HVSDK startup..."));

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString prefix_paths[] = {
		FPaths::GamePluginsDir(),
		FPaths::Combine(*FPaths::EnginePluginsDir(), TEXT("Marketplace/")),
	};

	FString dll_path;
	for (size_t i = 0; i < sizeof(prefix_paths) / sizeof(FString); i++)
	{
		FString dll_path_test = FPaths::Combine(*prefix_paths[i], DESKTOP_PLUGIN_DLL);
		if (FPaths::FileExists(dll_path_test)) {
			dll_path = dll_path_test;
			break;
		}
	}

	void *dll = FWindowsPlatformProcess::GetDllHandle(*dll_path);
	if (dll)
	{
#pragma warning(disable:4191)
		funcInit = (HvInit)FWindowsPlatformProcess::GetDllExport(dll, TEXT("HvInit"));
		funcDrive = (HvDrive)FWindowsPlatformProcess::GetDllExport(dll, TEXT("HvDrive"));
		funcGetDeviceInfo = (HvGetDeviceInfo)FWindowsPlatformProcess::GetDllExport(dll, TEXT("HvGetDeviceInfo"));
		funcGetUserInfo = (HvGetUserInfo)FWindowsPlatformProcess::GetDllExport(dll, TEXT("HvGetUserInfo"));
		funcGetOmniYawOffset = (HvGetOmniYawOffset)FWindowsPlatformProcess::GetDllExport(dll, TEXT("HvGetOmniYawOffset"));
		funcBuy = (HvBuy)FWindowsPlatformProcess::GetDllExport(dll, TEXT("HvBuy"));
		funcSendCommand = (HvSendCommand)FWindowsPlatformProcess::GetDllExport(dll, TEXT("HvSendCommand"));
		funcIsGuest = (HvIsGuest)FWindowsPlatformProcess::GetDllExport(dll, TEXT("HvIsGuest"));
		funcGetSysLang = (HvGetSysLang)FWindowsPlatformProcess::GetDllExport(dll, TEXT("HvGetSysLang"));
#pragma warning(default:4191)
		UE_LOG(LogTemp, Display, TEXT("HVSDK startup success!"));
	}

	if (NULL == funcInit) {
		UE_LOG(LogTemp, Error, TEXT("HVSDK load desktop_plugin.dll failed!"));
	}
}

void FOmniverseModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FOmniverseModule::PostLoadCallback()
{
	UE_LOG(LogTemp, Log, TEXT("FOmniverseModule::PostLoadCallback"));
}

void FOmniverseModule::PreUnloadCallback()
{
	UE_LOG(LogTemp, Log, TEXT("FOmniverseModule::PreUnloadCallback"));
}


TSharedPtr<class IInputDevice> FOmniverseModule::CreateInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler)
{
	UE_LOG(LogTemp, Warning, TEXT("Creating new Omni input Device"));
	OmniInputDevice = MakeShareable(new FOmniInputDevice(InMessageHandler));
	return TSharedPtr< class IInputDevice >(OmniInputDevice);

}


bool FWindowsHandler::ProcessMessage(HWND Hwnd, uint32 Message, WPARAM WParam, LPARAM LParam, int32& OutResult)
{
	if (Message == 0x0219)
	{
		switch ((int32)WParam)
		{
		case DeviceArrival: //0x8000
		case DeviceChanged: //0x0007
		{
			FOmniverseModule ref = FOmniverseModule::Get();
			if (ref.OmniInputDevice->tryingToReconnectOmni)
				return true;

			if (!ref.OmniInputDevice->OmniDisconnected)
				return true;

			if (ref.OmniInputDevice->tickerLoops >= 3)
				ref.OmniInputDevice->Init();

			return true;
		}
		default:
			return false;
		}
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FOmniverseModule, Omniverse)

