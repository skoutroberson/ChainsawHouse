// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Runtime/Engine/Classes/Engine/TargetPoint.h"
#include "TimerManager.h"
#include "ErrolController.generated.h"

/**
 * 
 */
UCLASS()
class CHAINSAWHOUSE_API AErrolController : public AAIController
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	class AErrolCharacter * ErrolCharacter;

	UPROPERTY()
		TArray<AActor*> Waypoints;

	UFUNCTION()
		ATargetPoint* GetRandomWaypoint();

	UFUNCTION()
		void GoToRandomWaypoint();

public:
		//virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;
		virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

		FTimerHandle LookAroundTimerHandle;
		void InitializeLookAroundTimer();
		//void StopLookAroundTimer();

	UFUNCTION()
		void LookAroundTimerCompleted();
		float LookAroundTimerRate = 3.5f;
		void SetLookAroundTimerRate(float Rate);

		void StopTimers();

		bool bTryToChase = false;

		FTimerHandle TryToChaseTimer;

		void TryToChase();

		AActor *Player = nullptr;

		// if peek scare level is below this then we will peek again
		// if this is greater than PeekScareThreshold then we will always keep peeking
		UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float PeekScareChaseAgainLvl = 0.0f;
};
