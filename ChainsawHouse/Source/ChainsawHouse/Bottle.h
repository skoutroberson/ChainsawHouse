// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Grabbable.h"
#include "ErrolCharacter.h"
#include "DestructibleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Bottle.generated.h"

/**
 * 
 */
UCLASS()
class CHAINSAWHOUSE_API ABottle : public AGrabbable
{
	GENERATED_BODY()

public:
	ABottle();

	//virtual void Tick(float DeltaTime) override;

	virtual void BeginPlay() override;

	UDestructibleComponent * DestructibleComponent = nullptr;
	UDestructibleMesh * DestructibleMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bottle)
	bool bStopSimulatingPhysics = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Bottle)
	USkeletalMeshComponent * AHC = nullptr;

private:
	UFUNCTION(BlueprintCallable, Category = ErrolCharacter)
	void NotifyErrolCharacter();

	AErrolCharacter * Errol = nullptr;

};
