// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Test3.generated.h"

UCLASS()
class SEMINAR_API ATest3 : public AActor
{
	GENERATED_BODY()

private:
	TArray<USplineMeshComponent*> SplineMeshComponents;
	
public:	
	// Sets default values for this actor's properties
	ATest3();

	USplineComponent* SplineComponent;

	float* OriginalImage = new float[1024 * 1024];

	float* StartImage = new float[1024 * 1024];

	float* EndImage = new float[1024 * 1024];

	float* FinalImage = new float[1024 * 1024];

	bool generateHeightmap = true;

	TArray<FVector2D> positions;

	TArray<FVector2D> minPos;

	TArray<FVector2D> allDirectionalPath;

	FVector2D highestPoint = FVector2D(0.0, 0.0);

	TArray<FVector2D> pointsAroundHighest;

	TArray<FVector> turns;

	TArray<FVector2D> turnTunnel;

	AActor* player;

	int currentPlayerIndex = 0;

	bool animation = true;

	int pointPrecision = 100;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual float getMapHeightAtPoint(FVector2D point);

	virtual FVector getUpVectorAtPoint(FVector2D point);

	virtual void saveImage(float* image, int imageSize, FString fileName, bool markLowest = false);

	virtual void generateImage(float* image, const int imageSize, FVector2D startLocation);

	virtual void generateCurvePoints(float* image, const int imageSize);

	virtual FVector getSplineLocationAtPosition(FVector2D position);

	virtual TArray<FVector> createPointsAtSpacing();

	virtual void initializeCurve();

	virtual void destroyOldSpline(bool destroyMesh);

	virtual void drawSpline();

	virtual void calculateHighestPoint();

	virtual void calculateTurn();

	virtual void allDirectionalMovement();

	virtual TArray<FVector2D> getPointsBetween(FVector2D start, FVector2D end);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
		UStaticMesh* Mesh;

	//UPROPERTY(EditAnywhere, Category = "Spline")
	//	USplineComponent* SplineComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
		FVector2D startPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
		FVector2D endPosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
		int turnCoefficient;

};
