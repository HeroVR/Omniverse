#pragma once
#include "Runtime/Core/Public/Core.h"
#include "Runtime/Engine/Public/EngineMinimal.h"
#include "HVDlgJson.h"
#include "HVMenuBox.generated.h"

UCLASS()
class OMNIVERSE_API AHVMenuBox : public AHVDlgJson
{
	GENERATED_UCLASS_BODY()

	~AHVMenuBox();
	static AHVMenuBox* GetInstance();

protected:
	static AHVMenuBox *Instance;

	virtual void EndPlay(const EEndPlayReason::Type reason) override;
};
