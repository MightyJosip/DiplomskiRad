// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Engine/SplineMeshActor.h"
#include "MySplineMeshActor.generated.h"

/**
 * 
 */
UCLASS()
class SEMINAR_API AMySplineMeshActor : public ASplineMeshActor
{
	GENERATED_BODY()	

public:
	// Sets default values for this actor's properties
	AMySplineMeshActor();

	float* OriginalImage = new float[1024 * 1024];

	float* StartImage = new float[1024 * 1024];

	float* EndImage = new float[1024 * 1024];

	float* FinalImage = new float[1024 * 1024];

	bool generateHeightmap = true;

	TArray<FVector2D> positions;

	// USplineComponent* SplineComponent;

	int pointPrecision = 100;

	TArray<USplineMeshComponent*> SplineMeshComponents;

	FVector2D startPosition = FVector2D(100, 200);

	FVector2D endPosition = FVector2D(900, 800);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// virtual void OnConstruction(const FTransform& Transform) override;

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

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spline")
	//	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, Category = "Spline")
		USplineComponent* SplineComponent;

};
