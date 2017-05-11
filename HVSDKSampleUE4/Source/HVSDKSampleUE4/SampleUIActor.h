// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "HvDlgBase.h"
#include "SampleUIActor.generated.h"

UCLASS()
class HVSDKSAMPLEUE4_API ASampleUIActor : public AHVDlgBase
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
	void OnClickButtonMsgBox();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	
	
};
