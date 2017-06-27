// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "OVDlgBase.h"
#include "GameFramework/SaveGame.h"
#include "SampleUIActor.generated.h"

UCLASS(Blueprintable, BlueprintType)
class USaveGameSample : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FTransform RedTransform;

	UPROPERTY(EditAnywhere)
	FTransform GreenTransform;
};

UCLASS()
class OVSDKSAMPLEUE4_API ASampleUIActor : public AOVDlgBase
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASampleUIActor(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnClickButtonPurchase();
	UFUNCTION()
	void OnPurchaseDone(FString itemName, FString outTradeNo, FString inTradeNo, FString errMsg);

	UFUNCTION()
	void OnClickButtonMsgBox();

	UFUNCTION()
	void OnClickButtonSaveGame();

	UFUNCTION()
	void OnClickButtonLoadGame();

	AOVDlgBase *DlgSaving, *DlgLoading;
	UFUNCTION()
	void OnSaveGameDone(int retCode, FString retMsg);

	UFUNCTION()
	void OnLoadGameDone(int retCode, FString retMsg, USaveGame *data);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
