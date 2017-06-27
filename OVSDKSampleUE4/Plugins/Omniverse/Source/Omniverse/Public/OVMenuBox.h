#pragma once
#include "Runtime/Core/Public/Core.h"
#include "Runtime/Engine/Public/EngineMinimal.h"
#include "OVDlgJson.h"
#include "OVMenuBox.generated.h"

UCLASS()
class OMNIVERSE_API AOVMenuBox : public AOVDlgJson
{
	GENERATED_UCLASS_BODY()

	~AOVMenuBox();
	static AOVMenuBox* GetInstance();

protected:
	static AOVMenuBox *Instance;
	float PreUserOmniCoupleRate;

	virtual void EndPlay(const EEndPlayReason::Type reason) override;
};
