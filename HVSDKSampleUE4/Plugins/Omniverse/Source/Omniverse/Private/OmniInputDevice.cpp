#include "OmniversePrivatePCH.h"
#include "OmniInputDevice.h"
#include "Features/IModularFeatures.h"

#define LOCTEXT_NAMESPACE "FOmniInputDevice"

const FName EOmniKeys::NAME_VirtuixCategory("Virtuix");

const FKey EOmniKeys::OmniInputX("OmniXAxis");
const FKey EOmniKeys::OmniInputY("OmniYAxis");
const FKey EOmniKeys::OmniInputYaw("OmniYaw");

int32 failedPackagesNum = 0;


FOmniInputDevice::FOmniInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) : MessageHandler(InMessageHandler)
{
	UE_LOG(LogTemp, Warning, TEXT("----------Current SDK Version - v1.4----------"));

	OmniHandle = nullptr;
	OmniDisconnected = true;
	tryingToReconnectOmni = false;
	NumFailedPackagesBeforeReconnect = 10;
	tickerAdded = false;
	tickerLoops = 0;

	AttemptToReconnectTheOmni_Handle = FDelegateHandle::FDelegateHandle();

	Ticker = FTicker::FTicker();

	PreInit();
	Init();
}

FOmniInputDevice::~FOmniInputDevice()
{
	bInitializationSucceeded = false;

	if (OmniHandle != nullptr)
	{
		hid_close(OmniHandle);
		hid_exit();
	}
}

void FOmniInputDevice::PreInit()
{

	// Register the FKeys
	EKeys::AddMenuCategoryDisplayInfo(EOmniKeys::NAME_VirtuixCategory, LOCTEXT("VirtuixSubCategory", "Virtuix"), TEXT("GraphEditor.PadEvent_16x"));

	EKeys::AddKey(FKeyDetails(EOmniKeys::OmniInputX, LOCTEXT("OmniXAxis", "Omni X-Axis"), FKeyDetails::FloatAxis, EOmniKeys::NAME_VirtuixCategory));
	EKeys::AddKey(FKeyDetails(EOmniKeys::OmniInputY, LOCTEXT("OmniYAxis", "Omni Y-Axis"), FKeyDetails::FloatAxis, EOmniKeys::NAME_VirtuixCategory));
	EKeys::AddKey(FKeyDetails(EOmniKeys::OmniInputYaw, LOCTEXT("OmniYaw", "Omni Yaw"), FKeyDetails::FloatAxis, EOmniKeys::NAME_VirtuixCategory));

	UE_LOG(LogTemp, Warning, TEXT("OmniInputDevice pre-init called"));
}

void FOmniInputDevice::Init(bool StartUpCall)
{
	tryingToReconnectOmni = true;
	bool bFoundHIDDevice = false;
	// Enumerate and print the HID devices on the system
	struct hid_device_info *devs, *cur_dev;
	FString serialNumber;
	FString path;
	devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;

	if (devs == NULL)
	{
		UE_LOG(LogTemp, Warning, TEXT("No HID Devices detected."));
		hid_exit();
		tryingToReconnectOmni = false;
		if (!tickerAdded)
			AttemptToReconnectTheOmni_Handle = Ticker.GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FOmniInputDevice::AttemptToReconnect), 0.5f);
		return;
	}


	if (CheckForOmniVersion(*cur_dev, cur_dev->path, "mi_04"))
	{
		serialNumber = cur_dev->serial_number;
		path = cur_dev->path;
		omni_dev = cur_dev;

		OmniHandle = hid_open_path(cur_dev->path);
		bFoundHIDDevice = true;
	}
	else
	{
		devs = hid_enumerate(0x0, 0x0);
		cur_dev = devs;

		if (CheckForOmniVersion(*cur_dev, cur_dev->path, "mi_00"))
		{
			serialNumber = cur_dev->serial_number;
			path = cur_dev->path;
			omni_dev = cur_dev;

			OmniHandle = hid_open_path(cur_dev->path);
			bFoundHIDDevice = true;
		}
	}

	if (bFoundHIDDevice)
	{
		hid_free_enumeration(devs);

		if (!OmniDisconnected)
		{
			tryingToReconnectOmni = false;
			Ticker.RemoveTicker(AttemptToReconnectTheOmni_Handle);
			tickerAdded = false;
			tickerLoops = 0;
			return;
		}

		// Set the hid_read() function to be non-blocking.
		hid_set_nonblocking(OmniHandle, 1);

		//hid_write(OmniHandle, initialDataBuffer, 65);
		bResetStepCount = true;
		bInitializationSucceeded = true;
		OmniDisconnected = false;
		tryingToReconnectOmni = false;
		tickerLoops = 0;
		UE_LOG(LogTemp, Warning, TEXT("Omni Found"));

		if (StartUpCall && !tickerAdded)
			AttemptToReconnectTheOmni_Handle = Ticker.GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FOmniInputDevice::AttemptToReconnect), 0.5f);

	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Omni Not Found"));
		tryingToReconnectOmni = false;

		if (!tickerAdded)
			AttemptToReconnectTheOmni_Handle = Ticker.GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FOmniInputDevice::AttemptToReconnect), 0.5f);
	}

	tryingToReconnectOmni = false;
}

void FOmniInputDevice::Tick(float DeltaTime)
{
	// Nothing to do when ticking, for now.  SendControllerEvents() handles everything.
}

void FOmniInputDevice::SendControllerEvents()
{
	if (!bInitializationSucceeded) return;

	omni_motion_t Omni_Internal_Motion;

	XAxis = 0.f;
	YAxis = 0.f;
	OmniYaw = 0.f;
	OmniTimeStamp = 0.f;

	OmniInputBuffer[0] = 0;

	if (OmniDisconnected)
	{
		MessageHandler->OnControllerAnalog("OmniYaw", 0, OmniYaw);
		MessageHandler->OnControllerAnalog("OmniXAxis", 0, XAxis);
		MessageHandler->OnControllerAnalog("OmniYAxis", 0, YAxis);

		UE_LOG(LogTemp, Warning, TEXT("Input zeroed out. Omni not connected."));
		return;
	}

	int32 readResult = hid_read(OmniHandle, OmniInputBuffer, 65);

	if (readResult == -1)
	{
		failedPackagesNum++;
		UE_LOG(LogTemp, Warning, TEXT("Number of Failed Omni Packages: %i"), failedPackagesNum);

		MessageHandler->OnControllerAnalog("OmniYaw", 0, OmniYaw);
		MessageHandler->OnControllerAnalog("OmniXAxis", 0, XAxis);
		MessageHandler->OnControllerAnalog("OmniYAxis", 0, YAxis);

		if (failedPackagesNum >= NumFailedPackagesBeforeReconnect)
		{
			failedPackagesNum = 0;
			OmniDisconnected = true;
			hid_close(OmniHandle);

			if (!tickerAdded)
				AttemptToReconnectTheOmni_Handle = Ticker.GetCoreTicker().AddTicker(FTickerDelegate::CreateRaw(this, &FOmniInputDevice::AttemptToReconnect), 0.5f);
		}

		UE_LOG(LogTemp, Error, TEXT("Omni is NOT receiving Input! Please make sure the Omni is connected and powered on!"));
		return;
	}


	//Data is not valid, but we don't want to disconnect the Omni. (Generally occurs when PODs are not streaming)
	if((OmniInputBuffer[0] != 0xEF && !OmniDisconnected) || (!OmniDisconnected && OmniInputBuffer[2] != 0xA9))	//Checking for Invalid Message
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Read HID Data. Please make sure the PODs are streaming."));

		MessageHandler->OnControllerAnalog("OmniYaw", 0, OmniYaw);
		MessageHandler->OnControllerAnalog("OmniXAxis", 0, XAxis);
		MessageHandler->OnControllerAnalog("OmniYAxis", 0, YAxis);

		return;
	}

	/* PAYLOAD DEFINITION:
	* Start at index 5 for most things:
	byte[0-3] = Timestamp (uint32)
	byte[4-7] = Step Count (uint32)
	byte[8-11] = Ring Angle (float32)
	byte[12] = Ring Delta (1 byte)
	byte[13-14] = Gamepad Data (2 bytes)
	byte[15] = L/R Step Trigger (1 byte)
	byte[16-23] = Pod 1 Quaternions (8 bytes, 2 bytes per)
	byte[24-29] = Pod 1 Accelerometer (6 bytes, 2 bytes per)
	byte[30-35] = Pod 1 Gyroscope (6 bytes, 2 bytes per)
	byte[36-43] = Pod 2 Quaternions (8 bytes, 2 bytes per)
	byte[44-49] = Pod 2 Accelerometer (6 bytes, 2 bytes per)
	byte[50-55] = Pod 2 Gyroscope (6 bytes, 2 bytes per)
	*/

	Omni_Internal_Motion.Timestamp = *(int *)&OmniInputBuffer[5 + 0];
	Omni_Internal_Motion.StepCount = *(int *)&OmniInputBuffer[5 + 4];

	OmniU.b[0] = OmniInputBuffer[5 + 8];
	OmniU.b[1] = OmniInputBuffer[5 + 9];
	OmniU.b[2] = OmniInputBuffer[5 + 10];
	OmniU.b[3] = OmniInputBuffer[5 + 11];

	Omni_Internal_Motion.RingAngle = OmniU.f;

	Omni_Internal_Motion.RingDelta = OmniInputBuffer[5 + 12];
	Omni_Internal_Motion.GameX = OmniInputBuffer[5 + 13];
	Omni_Internal_Motion.GameY = OmniInputBuffer[5 + 14];

	if (iCurrentTimeArrayIdx < 3)
	{
		iTimeArray[iCurrentTimeArrayIdx] = Omni_Internal_Motion.Timestamp;
		iCurrentTimeArrayIdx++;
	}
	else
	{
		iTimeArray[0] = iTimeArray[1];
		iTimeArray[1] = iTimeArray[2];
		iTimeArray[2] = iTimeArray[3];
		iTimeArray[3] = Omni_Internal_Motion.Timestamp;
	}

	if (iCurrentTimeArrayIdx == 3)
	{
		if ((iTimeArray[0] == iTimeArray[1])
			&& (iTimeArray[1] == iTimeArray[2])
			&& (iTimeArray[2] == iTimeArray[3]))
			bIsReceivingInput = false;
		else
			bIsReceivingInput = true;
	}

	if (bResetStepCount && bIsReceivingInput)
	{
		UE_LOG(LogTemp, Warning, TEXT("Internal Stepcount = %i"), Omni_Internal_Motion.StepCount);
		iStartingStepCount = Omni_Internal_Motion.StepCount;
		iCurrentStepCount = 0;
		bResetStepCount = false;
	}


	if (bIsReceivingInput)
	{
		XAxis = Omni_Internal_Motion.GameX;
		YAxis = Omni_Internal_Motion.GameY;
		iCurrentStepCount = Omni_Internal_Motion.StepCount - iStartingStepCount;
	}
	else
	{
		XAxis = 0;
		YAxis = 0;
	}
	OmniYaw = Omni_Internal_Motion.RingAngle;
	OmniTimeStamp = Omni_Internal_Motion.Timestamp;



	//

	if (bIsReceivingInput)
	{
		float rawX = XAxis;
		float rawY = YAxis;

		if (XAxis > 0.0)
		{
			XAxis = (XAxis / 255.0) * 2.0 - 1.0;
		}
		else
			XAxis = -1.0;

		if (YAxis > 0.0)
		{
			YAxis = (YAxis / 255.0) * 2.0 - 1.0;
		}
		else
			YAxis = -1.0;

		//clamp '0'
		if (rawX == 127)
		{
			XAxis = 0;
		}
		if (rawY == 127)
		{
			YAxis = 0;
		}

		YAxis *= -1.0;

		if (OmniYaw > 360.0)
			OmniYaw -= 360.0;
		else if (OmniYaw < 0.0)
			OmniYaw += 360.0;
	}

	//UE_LOG(LogTemp, Warning, TEXT("Yaw = %f"), OmniYaw);
	//UE_LOG(LogTemp, Warning, TEXT("X = %f"), XAxis);
	//UE_LOG(LogTemp, Warning, TEXT("Y = %f"), YAxis);


	MessageHandler->OnControllerAnalog("OmniYaw", 0, OmniYaw);
	MessageHandler->OnControllerAnalog("OmniXAxis", 0, XAxis);
	MessageHandler->OnControllerAnalog("OmniYAxis", 0, YAxis);

}

void FOmniInputDevice::SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	MessageHandler = InMessageHandler;
}

bool FOmniInputDevice::Exec(UWorld * InWorld, const TCHAR * Cmd, FOutputDevice & Ar)
{
	// No exec commands supported, for now.
	return false;
}


void FOmniInputDevice::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value) 
{
}

void FOmniInputDevice::SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values) 
{
}

bool FOmniInputDevice::CheckForOmniVersion(struct hid_device_info& cur_dev, char *path, char *OmniType)
{
	for (; *path; ++path) *path = tolower(*path);

	while (true)
	{
		path = strstr(cur_dev.path, "vid_29eb");

		if (path != NULL)
		{
			path = strstr(cur_dev.path, OmniType);

			if (path != NULL)
				return true;

			//return false;
		}

		if (cur_dev.next)
			cur_dev = *cur_dev.next;
		else
			break;
	}

	return false;
}

#undef LOCTEXT_NAMESPACE
