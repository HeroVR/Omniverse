#include "OmniversePrivatePCH.h"
#include "OmniInputDevice.h"
#include "Features/IModularFeatures.h"

#define LOCTEXT_NAMESPACE "FOmniInputDevice"

const FName EOmniKeys::NAME_VirtuixCategory("Virtuix");

const FKey EOmniKeys::OmniInputX("OmniXAxis");
const FKey EOmniKeys::OmniInputY("OmniYAxis");
const FKey EOmniKeys::OmniInputYaw("OmniYaw");



FOmniInputDevice::FOmniInputDevice(const TSharedRef<FGenericApplicationMessageHandler>& InMessageHandler) : MessageHandler(InMessageHandler)
{
	PreInit();
	Init();
}

FOmniInputDevice::~FOmniInputDevice()
{
	bInitializationSucceeded = false;

	hid_exit();
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

void FOmniInputDevice::Init()
{
	bool bFoundHIDDevice = false;
	// Enumerate and print the HID devices on the system
	struct hid_device_info *devs, *cur_dev;
	FString serialNumber;
	FString path;
	devs = hid_enumerate(0x0, 0x0);
	if (devs)
	{
		cur_dev = devs;

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
	}	

	if (bFoundHIDDevice)
	{
		UE_LOG(LogTemp, Warning, TEXT("Omni Found"));
		hid_free_enumeration(devs);

		// Set the hid_read() function to be non-blocking.
		hid_set_nonblocking(OmniHandle, 1);

		uint8 initialDataBuffer[65] = { 0 };
		uint8 logAllData[] = { 0xEF, 0x09, 0x93, 0x00, 0x00, 0x7F, 0x94, 0xA5, 0xBE };
		//CRC and settings for grabbing all data but step trigger and gun buttons
		uint8 logNoGunNoTrigger[] = { 0xEF, 0x09, 0x93, 0x00, 0x00, 0x1F, 0x94, 0x8D, 0xBE };

		initialDataBuffer[0] = 0;
		//memcpy(&initialDataBuffer[1], logAllData, 9);
		memcpy(&initialDataBuffer[1], logNoGunNoTrigger, 9);

		uint8 zeroByte = 0;

		hid_write(OmniHandle, initialDataBuffer, 65);
		bResetStepCount = true;
		bInitializationSucceeded = true;
		UE_LOG(LogTemp, Warning, TEXT("Omni Found"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Omni Not Found"));
	}
}

void FOmniInputDevice::Tick(float DeltaTime)
{
	// Nothing to do when ticking, for now.  SendControllerEvents() handles everything.
}

void FOmniInputDevice::SendControllerEvents()
{
#if UE_EDITOR
	if (!bInitializationSucceeded) 
	{
		RetryInitTimer += 0.02f;
		if (RetryInitTimer > 10.0f) 
		{
			RetryInitTimer = 0;
			Init();
		}
	}
#endif

	if (!bInitializationSucceeded) return;

	omni_motion_t Omni_Internal_Motion;

	XAxis = 0.f;
	YAxis = 0.f;
	OmniYaw = 0.f;
	OmniTimeStamp = 0.f;

	OmniInputBuffer[0] = 0;
	hid_read(OmniHandle, OmniInputBuffer, 65);

	Omni_Internal_Motion.Timestamp = *(int *)&OmniInputBuffer[6 + 0];
	Omni_Internal_Motion.StepCount = *(int *)&OmniInputBuffer[6 + 4];

	OmniU.b[0] = OmniInputBuffer[6 + 8];
	OmniU.b[1] = OmniInputBuffer[6 + 9];
	OmniU.b[2] = OmniInputBuffer[6 + 10];
	OmniU.b[3] = OmniInputBuffer[6 + 11];

	Omni_Internal_Motion.RingAngle = OmniU.f;

	Omni_Internal_Motion.RingDelta = OmniInputBuffer[6 + 12];
	Omni_Internal_Motion.GameX = OmniInputBuffer[6 + 13];
	Omni_Internal_Motion.GameY = OmniInputBuffer[6 + 14];
	//Omni_Internal_Motion.GunButtons = OmniInputBuffer[6 + 15]; No longer polling for this data
	//Omni_Internal_Motion.StepTrigger = OmniInputBuffer[6 + 16];



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