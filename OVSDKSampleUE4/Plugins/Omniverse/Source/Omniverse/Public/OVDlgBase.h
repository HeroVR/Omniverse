#pragma once
#include "Runtime/Core/Public/Core.h"
#include "Runtime/Engine/Public/EngineMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "WidgetComponent.h"
#include "OVDlgBase.generated.h"

class UUserWidget;
class AActor;
class AOVDlgBase;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDlgEvent, FString, sType, FString, sWidget);

UCLASS()
class OMNIVERSE_API AOVDlgBase : public AActor
{
	GENERATED_BODY()
	friend class UOVInterface;

public:
	AOVDlgBase(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay();

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 15
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
#else 
	virtual void EndPlay();
#endif
	virtual UWorld* GetWorld() const override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bAlwaysOnCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bBillboard;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoCloseTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutoFadeoutTime;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UWidgetComponent *WidgetComponent;

	UFUNCTION(BlueprintNativeEvent)
	void OnDlgEvent(const FString &type, const FString &param);

	UPROPERTY(BlueprintAssignable, Category = "OVMsgBox")
	FDlgEvent DlgEvent;

	UFUNCTION(BlueprintCallable)
	void SetAttribute(bool showRay, bool alwaysOnCamera = true, bool billboard = false, float duartion = 0, float fadeout = 0.5f);

	UFUNCTION(BlueprintCallable)
	void SetWidgetClass(TSubclassOf<UUserWidget> uiAsset);

	UFUNCTION(BlueprintCallable)
	UWidget* InitWidget(FName name, FText txt, bool visible = true);
protected:
	UUserWidget *RootWidget;
	int DlgIndex;

	float AutoCloseTimeLeft;

	virtual void Tick(float DeltaTime);
};
