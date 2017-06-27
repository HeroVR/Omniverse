#pragma once
#include "Runtime/Core/Public/Core.h"
#include "Runtime/Engine/Public/EngineMinimal.h"
#include "OVDlgBase.h"
#include "OVMsgBox.generated.h"

UENUM(BlueprintType)
namespace EMsgBoxFlag
{
	enum Flag
	{
		OkCancel,
		Ok,
		Cancel,
		None
	};
}

typedef void (*OVSDKMsgBoxCallback)(int32 btnIndex);

UCLASS()
class OMNIVERSE_API AOVMsgBox : public AOVDlgBase
{
	GENERATED_UCLASS_BODY()
	friend class UOVInterface;

	void Init(FText title, FText content, OVSDKMsgBoxCallback func, EMsgBoxFlag::Flag flag = EMsgBoxFlag::OkCancel);

protected:
	UButton *BtnYes;
	UButton *BtnCancel;
	UTextBlock *Title;
	UTextBlock *Content;

	UFUNCTION()
	void BtnYesClicked();
	UFUNCTION()
	void BtnCancelClicked();

	OVSDKMsgBoxCallback CallbackFunc;
};
