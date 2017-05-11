#pragma once
#include "Runtime/Core/Public/Core.h"
#include "Runtime/Engine/Public/EngineMinimal.h"
#include "HVDlgBase.h"
#include "HVMenuBox.generated.h"

class AHVMenuBox;

UCLASS()
class UMenuWidget : public UObject 
{
	GENERATED_BODY()

public:
	UWidget *Widget;
	FString Cmd;
	UTextBlock *TxtWidget;	
	AHVMenuBox *Dlg;

	UFUNCTION()
	void OnClick();
};

UCLASS()
class OMNIVERSE_API AHVMenuBox : public AHVDlgBase
{
	GENERATED_UCLASS_BODY()
	friend class UHvInterface;

	~AHVMenuBox();
	static AHVMenuBox* GetInstance();

protected:
	static AHVMenuBox *Instance;

	TArray<UMenuWidget*> _Widgets;
	bool NewWidget(UPanelWidget *panel, const FString &type, TSubclassOf<class UObject> clazz, int x, int y, int w, int h, const FString &style, const FString &cmd);
	bool UpdateText(UTextBlock *txtWidget, const FString &cmd);

	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type reason) override;
};
