// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Scare.h"
#include "Components/SplineComponent.h"
#include "StairsScare.generated.h"

/**
 * 
 */
UCLASS()
class CHAINSAWHOUSE_API AStairsScare : public AScare
{
	GENERATED_BODY()

	// spawn cralwer on the stairs and move up really fast with big stinger

protected:
	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable)
	void RotateMeshAlongSpline(float Time);

	USplineComponent *SplinePath = nullptr;
	USkeletalMeshComponent *Mesh = nullptr;
	
};
