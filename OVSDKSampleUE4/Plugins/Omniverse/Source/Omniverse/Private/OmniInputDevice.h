#pragma once
#include "OmniversePrivatePCH.h"
#include "OVDebugLogs.h"

#include "hidapi.h"


struct EOmniKeys
{
	//Name of category for the input pref
	static const FName NAME_VirtuixCategory;

	//Keys for the Omni
	static const FKey OmniInputX;
	static const FKey OmniInputY;
	static const FKey OmniInputYaw;
};

typedef struct motion_t {
	int Timestamp;
	int StepCount;
	float RingAngle = -1; 
	unsigned char RingDelta;
	unsigned char GameX;
	unsigned char GameY;
	unsigned char GunButtons;
	unsigned char StepTrigger;
} omni_motion_t;


class FOmniInputDevice : public IInputDevice
{
public:

	union {
		float f;
		unsigned char b[4];
	} OmniU;

	bool bInitializationSucceeded = false;

	omni_motion_t *m_OmniMotion;
	uint8 OmniInputBuffer[65] = { 0 };

	int iCurrentTimeArrayIdx = 0;
	int iTimeArray[4] = { 0 };
	bool bIsReceivingInput = true;

	float XAxis = 0;
	float YAxis = 0;
	float OmniYaw = 0;
	float OmniTimeStamp = 0;
	unsigned int iCurrentStepCount = 0;
	unsigned int iStartingStepCount = 0;
	bool bResetStepCount = true;

	hid_device *OmniHandle; 

	struct hid_device_info *omni_dev;

	bool OmniDisconnected;
	bool tryingToReconnectOmni;

	int32 NumFailedPackagesBeforeReconnect;

	FOmniInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler);
	~FOmniInputDevice();

	static void PreInit();
	void Init(bool StartUpCall = false);

	/** Tick the interface (e.g. check for new controllers) */
	virtual void Tick(float DeltaTime) override;

	/** Poll for controller state and send events if needed */
	virtual void SendControllerEvents() override;

	/** Set which MessageHandler will get the events from SendControllerEvents. */
	virtual void SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) override;

	/** Exec handler to allow console commands to be passed through for debugging */
	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar) override;
	
	/** IForceFeedbackSystem pass through functions */
	virtual void SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value);
	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values);

	bool CheckForOmniVersion(struct hid_device_info& cur_dev, char *path, char *OmniType);

	bool AttemptToReconnect(float param = 0)
	{
		tickerAdded = true;
		tickerTicking = true;

		if (OmniDisconnected && !tryingToReconnectOmni && (tickerLoops < 3))
		{
			Init();
			tickerLoops++;
		}
		else if (!OmniDisconnected || tickerLoops >= 3)
		{
			Ticker.GetCoreTicker().RemoveTicker(AttemptToReconnectTheOmni_Handle);
			tickerAdded = false;
			tickerTicking = false;
		}

		return true;
	}

	bool tickerTicking;
	int32 tickerLoops;
	bool tickerAdded;


private:
	/* Message handler */
	TSharedRef<FGenericApplicationMessageHandler> MessageHandler;

	FDelegateHandle AttemptToReconnectTheOmni_Handle;

	FTicker Ticker;

	OVDebug DebugFileRef;
	FString CommandLineArgs;
	bool PrintLogFile = false;
};
