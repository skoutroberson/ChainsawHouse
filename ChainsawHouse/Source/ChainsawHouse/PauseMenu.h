// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PauseMenu.generated.h"

UCLASS()
class CHAINSAWHOUSE_API APauseMenu : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APauseMenu();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void PauseMenuPressed();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UnPause();

	void CheckLocationAndRotation();

	bool bUpdateRotationAndLocation = false;
	void UpdateLocationAndRotation(float DeltaTime);

	USceneComponent *PlayerCamera = nullptr;

	UFUNCTION(BlueprintCallable)
	void PauseGame();

	UFUNCTION(BlueprintCallable)
	void UnPauseGame();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void PressYPaused();

	// set in beginplay
	class UStereoLayerComponent *StereoLayerComp = nullptr;

	USceneComponent *WidgetComp = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StereoLayerOffsetMultiplier = 10.0f;

};
