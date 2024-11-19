// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DogController.generated.h"

/**
 * 
 */
UCLASS()
class CHAINSAWHOUSE_API ADogController : public AAIController
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
public:
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	bool bMovingToUnderBall = false;

private:
	class ADog *Dog = nullptr;

	UWorld *World = nullptr;

public:
	bool bFailFlag = false;
	int FailCount = 0;

};
