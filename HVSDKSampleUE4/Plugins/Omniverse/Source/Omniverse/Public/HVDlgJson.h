#pragma once
#include "Runtime/Core/Public/Core.h"
#include "Runtime/Engine/Public/EngineMinimal.h"
#include "HVDlgBase.h"
#include "HVDlgJson.generated.h"

class AHVDlgJson;

UCLASS()
class UJsonWidget : public UObject 
{
	GENERATED_BODY()

public:
	UWidget *Widget;
	FString Name, Cmd;
	bool DynaText;
	UTextBlock *TxtWidget;	
	AHVDlgJson *Dlg;

	UFUNCTION()
	void OnClick();
};

UCLASS()
class OMNIVERSE_API AHVDlgJson : public AHVDlgBase
{
	GENERATED_UCLASS_BODY()
	friend class UHvInterface;

	~AHVDlgJson();

	bool LoadJson(const char *jsonFilePrefix);
	bool UpdateCmd(const FString &name, const FString &cmd);

	static char* LoadFile(const char *path);
protected:
	class UClass *ButtonClass, *TextClass;

	FString JsonPrefix;
	TArray<UJsonWidget*> JsonWidgets;

	bool NewWidget(UPanelWidget *panel, const FString &name, const FString &type, TSubclassOf<class UObject> clazz, int x, int y, int w, int h, const FString &style, const FString &cmd);
	bool UpdateText(UTextBlock *txtWidget, const FString &cmd);

	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type reason) override;
};
