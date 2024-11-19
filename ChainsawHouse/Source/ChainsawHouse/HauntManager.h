// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HauntManager.generated.h"

UENUM(BlueprintType)
enum class HauntManagerState : uint8
{
	STATE_IDLE					UMETA(DisplayName = "Idle"),
	STATE_TIMER					UMETA(DisplayName = "Timer"),
	STATE_HAUNTING				UMETA(DisplayName = "Haunting"),
	STATE_PAUSED				UMETA(DisplayName = "Paused"),
};

UENUM(BlueprintType)
enum class HauntState : uint8
{
	STATE_RANDOM				UMETA(DisplayName = "Random"),
	STATE_PEEKHAUNT				UMETA(DisplayName = "PeekHaunt"),
	STATE_SLOWHAUNT				UMETA(DisplayName = "SlowHaunt"),
	STATE_PEEKCHASEHAUNT		UMETA(DisplayName = "PeekChaseHaunt"),
};

UCLASS()
class CHAINSAWHOUSE_API AHauntManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHauntManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	HauntManagerState State;

	// keeps track of previous state when HauntManager is paused.
	UPROPERTY(VisibleAnywhere)
	HauntManagerState PauseState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	HauntState HState;

	UFUNCTION(BlueprintCallable)
	void StartHaunting(int HauntLevel, float MinTime, float MaxTime, HauntState HS = HauntState::STATE_RANDOM);

	UFUNCTION(BlueprintCallable)
	void StopHaunting();

	UFUNCTION(BlueprintCallable)
	void PauseHaunting();
	UFUNCTION(BlueprintCallable)
	void ResumeHaunting();


	/**
	1: Peek
	2: 
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int HauntLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HauntTimer = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HauntTime = 0.f;



	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinHauntTime = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxHauntTime = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HauntTimeOffset = 0.0f;



	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void StartHaunt();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void StopHaunt();

	// Sets HauntTime to a random float in the range MinHauntTime - MaxHauntTime.
	UFUNCTION(BlueprintCallable)
	void RollHauntTime();

};
