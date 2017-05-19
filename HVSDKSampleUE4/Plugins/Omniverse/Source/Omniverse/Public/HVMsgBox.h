#pragma once
#include "Runtime/Core/Public/Core.h"
#include "Runtime/Engine/Public/EngineMinimal.h"
#include "HVDlgBase.h"
#include "HVMsgBox.generated.h"

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

typedef void (*HVMSGBOX_CALLBACK)(int32 BtnIndex);

UCLASS()
class OMNIVERSE_API AHVMsgBox : public AHVDlgBase
{
	GENERATED_UCLASS_BODY()
	friend class UHvInterface;

	void Init(FText title, FText content, HVMSGBOX_CALLBACK func, EMsgBoxFlag::Flag flag = EMsgBoxFlag::OkCancel);

protected:
	UButton *BtnYes;
	UButton *BtnCancel;
	UTextBlock *Title;
	UTextBlock *Content;

	UFUNCTION()
	void BtnYesClicked();
	UFUNCTION()
	void BtnCancelClicked();

	HVMSGBOX_CALLBACK CallbackFunc;
};
