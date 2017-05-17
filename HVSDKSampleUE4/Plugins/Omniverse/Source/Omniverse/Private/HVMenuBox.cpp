#include "OmniversePrivatePCH.h"
#include "EngineGlobals.h"
#include "HVMenuBox.h"
#include "WidgetComponent.h"

AHVMenuBox *AHVMenuBox::Instance = NULL;

AHVMenuBox::AHVMenuBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LoadJson("systemmenu");

	if (!HasAnyFlags(RF_ClassDefaultObject)) {
		Instance = this;
	}
}

AHVMenuBox::~AHVMenuBox()
{
}

AHVMenuBox* AHVMenuBox::GetInstance() {
	return Instance;
}

void AHVMenuBox::EndPlay(const EEndPlayReason::Type reason)
{
	Super::EndPlay(reason);

	if (Instance == this) {
		Instance = nullptr;
	}	
}