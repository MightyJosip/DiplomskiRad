// Fill out your copyright notice in the Description page of Project Settings.


#include "Test2.h"
#include <Runtime/AssetRegistry/Public/AssetRegistry/AssetRegistryModule.h>
#include <GenericPlatform/GenericPlatformMath.h>

// Sets default values
ATest2::ATest2()
{
	PrimaryActorTick.bCanEverTick = true;
	SplineComponent = CreateDefaultSubobject<USplineComponent>("Spline");
}

// Called when the game starts or when spawned
void ATest2::BeginPlay()
{
	Super::BeginPlay();

	this->SetActorLocation(FVector(0, 0, 25));
	int imageSize = 1024;
	for (int32 y = 0; y < imageSize; y++)
	{
		for (int32 x = 0; x < imageSize; x++)
		{
			int map_X = -57100 + 111 * x;
			int map_Y = -57100 + 111 * y;
			float value = this->getMapHeightAtPoint(FVector2D(map_X, map_Y));
			int32 curPixelIndex = ((y * imageSize) + x);
			OriginalImage[curPixelIndex] = value;
		}
	}
	if (generateHeightmap)
	{
		this->saveImage(OriginalImage, imageSize, "Heightmap");
	}

	this->generateImage(StartImage, imageSize, startPosition);
	this->generateImage(EndImage, imageSize, endPosition);

	for (int32 y = 0; y < imageSize; y++)
	{
		for (int32 x = 0; x < imageSize; x++)
		{
			int32 curPixelIndex = ((y * imageSize) + x);
			FinalImage[curPixelIndex] = StartImage[curPixelIndex] + EndImage[curPixelIndex];
		}
	}

	this->generateCurvePoints(FinalImage, 1024);
	
	if (true)
	{
		this->saveImage(FinalImage, 1024, "FinalPath");
		this->saveImage(StartImage, 1024, "StartPath");
		this->saveImage(EndImage, 1024, "EndPath");
	}
	this->drawSpline();
}

float ATest2::getMapHeightAtPoint(FVector2D point)
{
	UWorld* world{ this->GetWorld() };
	if (point.X == 0) point.X = 1;
	if (point.Y == 0) point.Y = 1;
	if (world)
	{
		FVector startLocation{ point.X, point.Y, 30000 };
		FVector endLocation{ point.X, point.Y, -30000 };
		FHitResult hitResult;
		world->LineTraceSingleByObjectType(
			OUT hitResult,
			startLocation,
			endLocation,
			FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldStatic),
			FCollisionQueryParams()
		);
		if (hitResult.GetActor()) return hitResult.ImpactPoint.Z;
	}
	return 0;
}

void ATest2::generateImage(float* image, const int imageSize, FVector2D startLocation)
{
	for (int32 y = 0; y < imageSize; y++)
	{
		for (int32 x = 0; x < imageSize; x++)
		{
			int32 curPixelIndex = ((y * imageSize) + x);
			if (x != startLocation.X || y != startLocation.Y) image[curPixelIndex] = FLT_MAX;
			else image[curPixelIndex] = 0;
		}
	}
	for (int i = 0; i < 1; i++) {
		for (int32 y = 0; y < imageSize; y++)
		{
			for (int32 x = 0; x < imageSize; x++)
			{
				int32 curPixelIndex = ((y * imageSize) + x);
				float newValue = image[curPixelIndex];
				float selfValue = OriginalImage[curPixelIndex];
				float northWest = FLT_MAX; float north = FLT_MAX; float northEast = FLT_MAX; float west = FLT_MAX;
				if (x > 0 && y > 0) northWest = abs(OriginalImage[(y - 1) * imageSize + (x - 1)] - selfValue) + UE_SQRT_2 * 111 + image[(y - 1) * imageSize + (x - 1)];
				if (y > 0) north = abs(OriginalImage[(y - 1) * imageSize + x] - selfValue) + 111 + image[(y - 1) * imageSize + x];
				if (y > 0 && x < 1023)  northEast = abs(OriginalImage[(y - 1) * imageSize + (x + 1)] - selfValue) + UE_SQRT_2 * 111 + image[(y - 1) * imageSize + (x + 1)];
				if (x > 0) west = abs(OriginalImage[y * imageSize + (x - 1)] - selfValue) + 111 + image[y * imageSize + (x - 1)];
				image[curPixelIndex] = FMath::Min(newValue, FMath::Min(northWest, FMath::Min(north, FMath::Min(northEast, west))));
			}
		}
		for (int32 y = imageSize - 1; y >= 0; y--)
		{
			for (int32 x = imageSize - 1; x >= 0; x--)
			{
				int32 curPixelIndex = ((y * imageSize) + x);
				float newValue = image[curPixelIndex];
				float selfValue = OriginalImage[curPixelIndex];
				float southWest = FLT_MAX; float south = FLT_MAX; float southEast = FLT_MAX; float east = FLT_MAX;
				if (x < 1023 && y < 1023) southEast = abs(OriginalImage[(y + 1) * imageSize + (x + 1)] - selfValue) + UE_SQRT_2 * 111 + image[(y + 1) * imageSize + (x + 1)];
				if (y < 1023) south = abs(OriginalImage[(y + 1) * imageSize + x] - selfValue) + 111 + image[(y + 1) * imageSize + x];
				if (y < 1023 && x > 0) southWest = abs(OriginalImage[(y + 1) * imageSize + (x - 1)] - selfValue) + UE_SQRT_2 * 111 + image[(y + 1) * imageSize + (x - 1)];
				if (x < 1023) east = abs(OriginalImage[y * imageSize + (x + 1)] - selfValue) + 111 + image[y * imageSize + (x + 1)];
				image[curPixelIndex] = FMath::Min(newValue, FMath::Min(southEast, FMath::Min(south, FMath::Min(southWest, east))));
			}
		}
	}
}

void ATest2::saveImage(float* image, const int imageSize, const FString fileName, bool markLowest)
{
	uint8* Pixels = new uint8[1024 * 1024 * 4];
	float min = FLT_MAX, max = FLT_MIN;
	for (int32 y = 0; y < imageSize; y++)
	{
		for (int32 x = 0; x < imageSize; x++)
		{
			int32 curPixelIndex = ((y * imageSize) + x);
			if (image[curPixelIndex] < min) min = image[curPixelIndex];
			if (image[curPixelIndex] > max) max = image[curPixelIndex];
		}
	}
	for (int32 y = 0; y < imageSize; y++)
	{
		for (int32 x = 0; x < imageSize; x++)
		{
			int32 curPixelIndex = ((y * imageSize) + x);
			uint8 new_value = 255 * ((image[curPixelIndex] - min) / (max - min));
			if (x == startPosition.X && y == startPosition.Y)
			{
				Pixels[4 * curPixelIndex] = 255;
				Pixels[4 * curPixelIndex + 1] = 0;
				Pixels[4 * curPixelIndex + 2] = 0;
			}
			else if (x == endPosition.X && y == endPosition.Y)
			{
				Pixels[4 * curPixelIndex] = 0;
				Pixels[4 * curPixelIndex + 1] = 255;
				Pixels[4 * curPixelIndex + 2] = 0;
			}
			else
			{
				Pixels[4 * curPixelIndex] = new_value;
				Pixels[4 * curPixelIndex + 1] = new_value;
				Pixels[4 * curPixelIndex + 2] = new_value;
			}
		}
	}
	if (markLowest)
	{
		for (FVector2D pos : positions)
		{
			int32 curPixelIndex = ((static_cast<int>(pos.Y) * imageSize) + static_cast<int>(pos.X));
			Pixels[4 * curPixelIndex + 1] = 255;  Pixels[4 * curPixelIndex + 2] = 255;
		}
	}
	FString PackageName = TEXT("/Game/ProceduralTextures/");
	PackageName += fileName;
	UPackage* Package = CreatePackage(NULL, *PackageName);
	Package->FullyLoad();

	UTexture2D* NewTexture = NewObject<UTexture2D>(Package, *fileName, RF_Public | RF_Standalone | RF_MarkAsRootSet);
	NewTexture->AddToRoot();				// This line prevents garbage collection of the texture
	NewTexture->PlatformData = new FTexturePlatformData();	// Then we initialize the PlatformData
	NewTexture->PlatformData->SizeX = imageSize;
	NewTexture->PlatformData->SizeY = imageSize;
	NewTexture->PlatformData->SetNumSlices(1);
	NewTexture->PlatformData->PixelFormat = EPixelFormat::PF_B8G8R8A8;

	FTexture2DMipMap* Mip = new(NewTexture->PlatformData->Mips) FTexture2DMipMap();
	Mip->SizeX = 1143;
	Mip->SizeY = 1143;

	// Lock the texture so it can be modified
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	uint8* TextureData = (uint8*)Mip->BulkData.Realloc(imageSize * imageSize * 4);
	FMemory::Memcpy(TextureData, Pixels, sizeof(uint8) * imageSize * imageSize * 4);
	Mip->BulkData.Unlock();

	NewTexture->Source.Init(imageSize, imageSize, 1, 1, ETextureSourceFormat::TSF_BGRA8, Pixels);

	NewTexture->UpdateResource();
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(NewTexture);

	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	bool bSaved = UPackage::SavePackage(Package, NewTexture, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *PackageFileName, GError, nullptr, true, true, SAVE_NoError);
}


void ATest2::generateCurvePoints(float* image, const int imageSize)
{
	int pos_x = startPosition.X; int pos_y = startPosition.Y;
	FVector2D directions[8]
	{
		{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
	};
	int last_dir = -1;
	bool dir_changed = true;
	// for (int i = 0; i < 700; i++)
	while (pos_x != endPosition.X || pos_y != endPosition.Y)
	{
		int min_x, min_y, min_dir;
		int32 curPixelIndex = ((pos_y * imageSize) + pos_x);
		if (dir_changed)
		{
			//if (last_dir == -1)
			//{
			//	positions.Add(FVector2D(pos_x, pos_y));
			//}
			//else
			//{
				// positions.Add(FVector2D(pos_x - static_cast<int>(directions[last_dir].X), pos_y - static_cast<int>(directions[last_dir].Y)));
			//}
		}
		if (last_dir != -1) positions.Add(FVector2D(pos_x - static_cast<int>(directions[last_dir].X), pos_y - static_cast<int>(directions[last_dir].Y)));
		else positions.Add(FVector2D(startPosition.X, startPosition.Y));
		dir_changed = false;
		float min = FLT_MAX;
		if (last_dir == -1)
		{
			for (int j = 0; j < 8; j++)
			{
				curPixelIndex = (((pos_y + static_cast<int>(directions[j].Y)) * imageSize) + (pos_x + static_cast<int>(directions[j].X)));
				float value = image[curPixelIndex];
				if ((value - min) < 0)
				{
					min = value;
					min_x = pos_x + static_cast<int>(directions[j].X);
					min_y = pos_y + static_cast<int>(directions[j].Y);
					min_dir = j;
				}
			}

		}
		else
		{
			for (int j = -1; j < 2; j++)
			{
				int dir = last_dir + j;
				if (last_dir == 0 && j == -2) dir = 6;
				if (last_dir == 0 && j == -1) dir = 7;
				if (last_dir == 1 && j == -2) dir = 7;
				curPixelIndex = (((pos_y + static_cast<int>(directions[dir].Y)) * imageSize) + (pos_x + static_cast<int>(directions[dir].X)));
				float value = image[curPixelIndex];
				if ((value - min + abs(j)) < 0)
				{
					min = value;
					min_x = pos_x + static_cast<int>(directions[dir].X);
					min_y = pos_y + static_cast<int>(directions[dir].Y);
					min_dir = dir;
				}
			}
		}
		if (last_dir != min_dir)
		{
			last_dir = min_dir;
			dir_changed = true;
		}
		last_dir = min_dir;
		pos_x = min_x;
		pos_y = min_y;
	}
	positions.Add(FVector2D(endPosition.X, endPosition.Y));
}

FVector ATest2::getSplineLocationAtPosition(FVector2D position)
{
	return FVector(position.X, position.Y, this->getMapHeightAtPoint(position));
}

TArray<FVector> ATest2::createPointsAtSpacing()
{
	TArray<FVector> locationsToAdd;
	float currentDistance = 0;
	float totalDistance = SplineComponent->GetSplineLength();
	int numberOfPoints = totalDistance / pointPrecision;
	for (int index = 0; index <= numberOfPoints; index++)
	{
		FVector oldPosition = SplineComponent->GetLocationAtDistanceAlongSpline(currentDistance, ESplineCoordinateSpace::Local);
		//UE_LOG(LogTemp, Warning, TEXT("POINT IN LOOP %f %f"), oldPosition.X, oldPosition.Y);
		locationsToAdd.Add(this->getSplineLocationAtPosition(FVector2D(oldPosition.X, oldPosition.Y)));
		currentDistance += pointPrecision;
	}
	FVector endPoint = SplineComponent->GetLocationAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::Local);
	locationsToAdd.Add(this->getSplineLocationAtPosition(FVector2D(endPoint.X, endPoint.Y)));
	return locationsToAdd;
}

void ATest2::destroyOldSpline(bool destroyMesh)
{
	for (int index = SplineComponent->GetNumberOfSplinePoints() - 1; index >= 0; index--)
	{
		SplineComponent->RemoveSplinePoint(index);
	}
	if (destroyMesh)
	{
		for (int index = 0; index < SplineMeshComponents.Num(); index++)
		{
			SplineMeshComponents[index]->DestroyComponent();
		}
		SplineMeshComponents.Empty();
	}
}

void ATest2::initializeCurve()
{
	SplineMeshComponents.Empty();
	for (int index = 1; index < positions.Num(); index++)
	{
		SplineComponent->AddSplinePoint(FVector(-57100 + 111 * positions[index].X, -57100 + 111 * positions[index].Y, 10), ESplineCoordinateSpace::Local);
		//UE_LOG(LogTemp, Warning, TEXT("POINT BEFORE %f %f"), -57100 + 111 * positions[index].X, -57100 + 111 * positions[index].Y);
	}
	TArray<FVector> locationsToAdd = this->createPointsAtSpacing();
	this->destroyOldSpline(false);
	for (int index = 0; index < locationsToAdd.Num(); index++)
	{
		SplineComponent->AddSplinePoint(FVector(locationsToAdd[index].X, locationsToAdd[index].Y, locationsToAdd[index].Z), ESplineCoordinateSpace::Local, true);
	}
}

FVector ATest2::getUpVectorAtPoint(FVector2D point)
{
	UWorld* world{ this->GetWorld() };
	if (point.X == 0) point.X = 1;
	if (point.Y == 0) point.Y = 1;
	if (world)
	{
		FVector startLocation{ point.X, point.Y, 5000 };
		FVector endLocation{ point.X, point.Y, -5000 };
		FHitResult hitResult;
		world->LineTraceSingleByObjectType(
			OUT hitResult,
			startLocation,
			endLocation,
			FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldStatic),
			FCollisionQueryParams()
		);
		if (hitResult.GetActor()) return hitResult.ImpactNormal;
	}
	return FVector(0, 0, 0);
}

void ATest2::drawSpline()
{
	if (!Mesh)
	{
		return;
	}
	int spacing = 500;
	int line_width = 500;
	//UE_LOG(LogTemp, Warning, TEXT("DRAWING"));
	this->DestroyConstructedComponents();
	this->destroyOldSpline(true);
	this->initializeCurve();
	float curveLength = SplineComponent->GetSplineLength();
	//UE_LOG(LogTemp, Warning, TEXT("LENGTH %f"), curveLength);
	int numberOfMeshes = (curveLength - fmod(curveLength, spacing)) / spacing;
	float scaleFactor = line_width / Mesh->GetBoundingBox().GetSize().X;
	for (int index = 0; index < numberOfMeshes + 1; index++)
	{
		USplineMeshComponent* splineMeshComponent = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		SplineMeshComponents.Add(splineMeshComponent);
		splineMeshComponent->SetStaticMesh(Mesh);

		FVector startLocation = SplineComponent->GetLocationAtDistanceAlongSpline(index * spacing, ESplineCoordinateSpace::Local);
		FVector startTangent = SplineComponent->GetDirectionAtDistanceAlongSpline(index * spacing, ESplineCoordinateSpace::Local);

		FVector endLocation = SplineComponent->GetLocationAtDistanceAlongSpline((index + 1) * spacing, ESplineCoordinateSpace::Local);
		FVector endTangent = SplineComponent->GetDirectionAtDistanceAlongSpline((index + 1) * spacing, ESplineCoordinateSpace::Local);

		splineMeshComponent->SetStartAndEnd(startLocation, startTangent, endLocation, endTangent);
		// UE_LOG(LogTemp, Warning, TEXT("CREATING PATH %f %f"), startLocation.X, startLocation.Y);
		splineMeshComponent->SetSplineUpDir(this->getUpVectorAtPoint(FVector2D(startLocation.X, startLocation.Y)), true);

		splineMeshComponent->SetStartScale(FVector2D(scaleFactor, scaleFactor), true);
		splineMeshComponent->SetEndScale(FVector2D(scaleFactor, scaleFactor), true);
		splineMeshComponent->SetMobility(EComponentMobility::Movable);
		splineMeshComponent->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepRelativeTransform);
		splineMeshComponent->RegisterComponent();
		this->AddInstanceComponent(splineMeshComponent);
	}
	
}


// Called every frame
void ATest2::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

