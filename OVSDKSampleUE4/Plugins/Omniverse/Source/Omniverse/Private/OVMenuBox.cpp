#include "OmniversePrivatePCH.h"
#include "EngineGlobals.h"
#include "OVMenuBox.h"
#include "WidgetComponent.h"

AOVMenuBox *AOVMenuBox::Instance = NULL;

AOVMenuBox::AOVMenuBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), PreUserOmniCoupleRate(-1)
{
	LoadJson("systemmenu");

	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		Instance = this;
		
		UJsonWidget *widgetCoupleRate = LookupWidget("OmniCoupleRate");
		if (widgetCoupleRate != nullptr) 
		{
			PreUserOmniCoupleRate = UOVInterface::GetOmniCoupleRate();
			((USlider*)widgetCoupleRate->Widget->GetRootWidget())->SetValue(PreUserOmniCoupleRate);
		}
	}
}

AOVMenuBox::~AOVMenuBox()
{
}

AOVMenuBox* AOVMenuBox::GetInstance() {
	return Instance;
}

void AOVMenuBox::EndPlay(const EEndPlayReason::Type reason)
{
	Super::EndPlay(reason);

	if (PreUserOmniCoupleRate > -0.01f 
		&& UOVInterface::GetUserInfo()->nUserCoupleRate > 0 
		&& PreUserOmniCoupleRate != (0.0001 * UOVInterface::GetUserInfo()->nUserCoupleRate - 1)) {
		UOVInterface::SendCommand(14, "", 0);
	}

	if (Instance == this) {
		Instance = nullptr;
	}	
}