// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Scare.generated.h"

UCLASS()
class CHAINSAWHOUSE_API AScare : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AScare();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Scare")
	void StartScare();
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Scare")
	void EndScare();

protected:

	virtual void StartScareInternal() {}
	virtual void EndScareInternal() {}

};
