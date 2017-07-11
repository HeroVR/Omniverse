#pragma once
#include "Runtime/Core/Public/Core.h"
#include "Runtime/Engine/Public/EngineMinimal.h"
#include "OVDlgBase.h"
#include "OVDlgJson.generated.h"

class AOVDlgJson;

UCLASS()
class UJsonWidget : public UObject 
{
	GENERATED_BODY()

public:
	UUserWidget *Widget;
	FString Name, Cmd;
	int MsgId;
	bool DirectClose, DynaText;
	UTextBlock *TxtWidget;	
	AOVDlgJson *Dlg;

	UFUNCTION()
	void OnClick();

	UFUNCTION()
	void OnSlide(float val);

	UFUNCTION()
	void OnToggle(bool state);
};

UCLASS()
class OMNIVERSE_API AOVDlgJson : public AOVDlgBase
{
	GENERATED_UCLASS_BODY()
	friend class UOVInterface;

	~AOVDlgJson();

	bool LoadJson(const char *jsonFilePrefix);
	bool UpdateCmd(const FString &name, const FString &cmd);
	void TryClose(const FString &name);
	inline const FString &GetDlgJsonName() const {
		return DlgJsonName;
	}

	static char* LoadFile(const char *path);
protected:
	class UClass *ButtonClass, *TextClass, *SliderClass, *ToggleClass;
	float PreUserOmniCoupleRate;

	FString JsonPrefix;
	FString DlgJsonName;
	TArray<UJsonWidget*> JsonWidgets;

	bool NewWidget(UPanelWidget *panel, const FString &name, const FString &type, int x, int y, int w, int h, int MsgId, bool DirectClose, const FString &style, const FString &cmd);
	UJsonWidget* LookupWidget(FString name);
	bool UpdateText(UTextBlock *txtWidget, const FString &cmd);

	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type reason) override;
};
