// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Radio.generated.h"

UCLASS()
class CHAINSAWHOUSE_API ARadio : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARadio();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Radio")
	void TurnOn();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Radio")
	void TurnOff();

	UWorld *World = nullptr;
	class UNavigationSystemV1 * NavigationSystem = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent *Root = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent *SoundLocation = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent *PathStart = nullptr;

	void UpdateSoundLocation(float DeltaTime);

	AActor * Player = nullptr;

	USceneComponent *PlayerCamera = nullptr;

	class UAudioComponent *AC = nullptr;
	class USoundAttenuation *Att = nullptr;

	int CurrentPathPointsNum = 1;

	// volume multiplier for the audio component
	float CurrentAudioVolume = 1.0f;

	float CurrentStereoSpread = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UCurveFloat *DistanceFadeCurve = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UCurveFloat *FadeCurveCorrector = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UCurveFloat *RadioReverbDist = nullptr;

	void SimulateReverb(float DeltaTime);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ReverbDistance = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AudioMoveSpeed = 720.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AudioFadeSpeed = 0.3f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float CurrentReverbValue = 0.0f;

	FVector LastPlayerLocation = FVector::ZeroVector;

	TArray<FVector> NavPathPoints;

	void GetNewPath();
};
