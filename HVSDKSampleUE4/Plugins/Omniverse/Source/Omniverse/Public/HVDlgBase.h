#pragma once
#include "Runtime/Core/Public/Core.h"
#include "Runtime/Engine/Public/EngineMinimal.h"
#include "Runtime/Launch/Resources/Version.h"
#include "WidgetComponent.h"
#include "HVDlgBase.generated.h"

class UUserWidget;
class AActor;
class AHVDlgBase;
class UButton;

UCLASS()
class OMNIVERSE_API AHVDlgBase : public AActor
{
	GENERATED_BODY()
	friend class UHvInterface;

public:
	AHVDlgBase(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay();

#if ENGINE_MAJOR_VERSION >= 4 && ENGINE_MINOR_VERSION >= 15
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason);
#else 
	virtual void EndPlay();
#endif
	virtual UWorld* GetWorld() const override;
	
	UPROPERTY(EditAnywhere)
	bool bIsStatic;
	UPROPERTY(EditAnywhere)
	bool bShowRay;

	UPROPERTY(EditAnywhere)
	bool bAlwaysOnCamera;

	UPROPERTY(BlueprintReadWrite)
	UWidgetComponent *WidgetComponent;


	UFUNCTION(BlueprintImplementableEvent)
	void OnDlgEvent(const FString &type, const FString &param);

protected:
	UUserWidget *RootWidget;
	int DlgIndex;

	UButton* InitButton(const char *name, FText txt, bool visible = true);

	virtual void Tick(float DeltaTime);
};
