// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grabbable.h"
#include "LightManager.h"
#include "Flashlight.generated.h"

/**
 * 
 */
UCLASS()
class CHAINSAWHOUSE_API AFlashlight : public AGrabbable
{
	GENERATED_BODY()

public:
	AFlashlight();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void TurnOn();
	UFUNCTION(BlueprintCallable)
	void TurnOff();

	UFUNCTION(BlueprintCallable)
	void PressButton(bool bButtonAudio);

	UPROPERTY(BlueprintReadWrite)
	bool bOn = false;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UStaticMeshComponent * Mesh = nullptr;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class UAudioComponent * ButtonPressAudio = nullptr;

	UFUNCTION(BlueprintImplementableEvent)
	void PlayButtonPressAudio();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bSpotlightRevealEnabled = true;

	UPROPERTY(BlueprintReadWrite)
	ALightManager * LM = nullptr;

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateLightVolumetrics(bool bLightIsOn);

	virtual void Gripped(int HandHoldNum) override;

	virtual void Released(int HandHoldNum) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bSpawnFlicker = true;

	// flicker the light after a short delay if they player drops it (in case it is in the dark)
	UFUNCTION(BlueprintImplementableEvent)
	void FlickerAfterDelay();
	
};
