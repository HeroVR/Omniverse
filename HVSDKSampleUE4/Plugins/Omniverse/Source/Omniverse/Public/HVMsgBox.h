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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMsgBoxEvent, FString, sType, FString, sWidget);

UCLASS()
class OMNIVERSE_API AHVMsgBox : public AHVDlgBase
{
	GENERATED_UCLASS_BODY()
	friend class UHvInterface;

	void Init(FText title, FText content, HVMSGBOX_CALLBACK func, EMsgBoxFlag::Flag flag = EMsgBoxFlag::OkCancel);

	UPROPERTY(BlueprintAssignable, Category = "HVMsgBox")
	FMsgBoxEvent MsgBoxEvent;

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
