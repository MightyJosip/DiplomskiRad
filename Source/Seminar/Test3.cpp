// Fill out your copyright notice in the Description page of Project Settings.


#include "Test3.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ATest3::ATest3()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SplineComponent = CreateDefaultSubobject<USplineComponent>("Spline");

}

// Called when the game starts or when spawned
void ATest3::BeginPlay()
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

	//this->calculateHighestPoint();

	//this->calculateTurn();

	//this->allDirectionalMovement();

	if (true)
	{
		this->saveImage(FinalImage, 1024, "FinalPath", true);
		//this->saveImage(OriginalImage, 1024, "FinalPath", true);
		this->saveImage(StartImage, 1024, "StartPath", false);
		this->saveImage(EndImage, 1024, "EndPath", false);
	}
	this->drawSpline();

	TArray<AActor*> playerArray;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), playerArray);
	UE_LOG(LogTemp, Warning, TEXT("Players found %d"), playerArray.Num());
	player = playerArray[0];
	player->SetActorLocation(FVector(-57100 + 111 * startPosition.X, -57100 + 111 * startPosition.Y, 200 + getMapHeightAtPoint(FVector2D(-57100 + 111 * startPosition.X, -57100 + 111 * startPosition.Y))));
}

void ATest3::generateImage(float* image, const int imageSize, FVector2D startLocation)
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
	float diagonal_move = UE_SQRT_2 * 111;
	for (int i = 0; i < 1; i++) 
	{
		for (int32 y = 0; y < imageSize; y++)
		{
			for (int32 x = 0; x < imageSize; x++)
			{
				int32 curPixelIndex = ((y * imageSize) + x);
				float selfValue = OriginalImage[curPixelIndex];
				float northWest = FLT_MAX; float north = FLT_MAX; float northEast = FLT_MAX; float west = FLT_MAX;
				if (x > 0 && y > 0) northWest = abs(OriginalImage[(y - 1) * imageSize + (x - 1)] - selfValue) + diagonal_move + image[(y - 1) * imageSize + (x - 1)];
				if (y > 0) north = abs(OriginalImage[(y - 1) * imageSize + x] - selfValue) + 111 + image[(y - 1) * imageSize + x];
				if (y > 0 && x < 1023)  northEast = abs(OriginalImage[(y - 1) * imageSize + (x + 1)] - selfValue) + diagonal_move + image[(y - 1) * imageSize + (x + 1)];
				if (x > 0) west = abs(OriginalImage[y * imageSize + (x - 1)] - selfValue) + 111 + image[y * imageSize + (x - 1)];
				image[curPixelIndex] = FMath::Min(image[curPixelIndex], FMath::Min(northWest, FMath::Min(north, FMath::Min(northEast, west))));
			}
		}
		for (int32 y = imageSize - 1; y >= 0; y--)
		{
			for (int32 x = imageSize - 1; x >= 0; x--)
			{
				int32 curPixelIndex = ((y * imageSize) + x);
				float selfValue = OriginalImage[curPixelIndex];
				float southWest = FLT_MAX; float south = FLT_MAX; float southEast = FLT_MAX; float east = FLT_MAX;
				if (x < 1023 && y < 1023) southEast = abs(OriginalImage[(y + 1) * imageSize + (x + 1)] - selfValue) + diagonal_move + image[(y + 1) * imageSize + (x + 1)];
				if (y < 1023) south = abs(OriginalImage[(y + 1) * imageSize + x] - selfValue) + 111 + image[(y + 1) * imageSize + x];
				if (y < 1023 && x > 0) southWest = abs(OriginalImage[(y + 1) * imageSize + (x - 1)] - selfValue) + diagonal_move + image[(y + 1) * imageSize + (x - 1)];
				if (x < 1023) east = abs(OriginalImage[y * imageSize + (x + 1)] - selfValue) + 111 + image[y * imageSize + (x + 1)];
				image[curPixelIndex] = FMath::Min(image[curPixelIndex], FMath::Min(southEast, FMath::Min(south, FMath::Min(southWest, east))));
			}
		}
	}
	
}

float ATest3::getMapHeightAtPoint(FVector2D point)
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

void ATest3::saveImage(float* image, const int imageSize, const FString fileName, bool markLowest)
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
			else if (allDirectionalPath.Contains(FVector2D(x, y)))
			{
				Pixels[4 * curPixelIndex] = 255;
				Pixels[4 * curPixelIndex + 1] = 0;
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
		/*int32 startIndex = ((startPosition.Y * imageSize) + startPosition.X);
		float min_value = image[startIndex];
		for (int y = 0; y < 1024; y++)
		{
			for (int x = 0; x < 1024; x++)
			{
				int32 curPixelIndex = ((y * imageSize) + x);
				if (image[curPixelIndex] - 2 < min_value)
				{
					Pixels[4 * curPixelIndex + 1] = 255;  Pixels[4 * curPixelIndex + 2] = 255;
				}
			}
		}*/
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

void ATest3::calculateHighestPoint()
{
	float maxHeight = -FLT_MAX;
	for (FVector2D vec : positions)
	{
		float pointHeight = this->getMapHeightAtPoint(FVector2D(-57100 + 111 * vec.X, -57100 + 111 * vec.Y));
		if (pointHeight > maxHeight)
		{
			highestPoint.Set(vec.X, vec.Y);
			maxHeight = pointHeight;
		}
	}
	pointsAroundHighest.Add(highestPoint);

	FVector2D directions[8]
	{
		{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
	};
	int direction = 0;
	FVector2D nextPoint = FVector2D(highestPoint.X, highestPoint.Y);
	for (int i = 0; i < 8; i++)
	{
		nextPoint.Set(highestPoint.X + directions[i].X, highestPoint.Y + directions[i].Y);
		if (positions.Contains(nextPoint) && this->getMapHeightAtPoint(FVector2D(-57100 + 111 * nextPoint.X, -57100 + 111 * nextPoint.Y)) < this->getMapHeightAtPoint(FVector2D(-57100 + 111 * highestPoint.X, -57100 + 111 * highestPoint.Y)) + 1)
		{
			pointsAroundHighest.Add(FVector2D(nextPoint.X, nextPoint.Y));
			while (true)
			{
				FVector2D anotherPoint = FVector2D(nextPoint.X + directions[i].X, nextPoint.Y + directions[i].Y);
				if (positions.Contains(anotherPoint) && this->getMapHeightAtPoint(FVector2D(-57100 + 111 * anotherPoint.X, -57100 + 111 * anotherPoint.Y)) <= this->getMapHeightAtPoint(FVector2D(-57100 + 111 * nextPoint.X, -57100 + 111 * nextPoint.Y)) + 1)
				{
					nextPoint = anotherPoint;
					pointsAroundHighest.Add(FVector2D(nextPoint.X, nextPoint.Y));
				}
				else
				{
					break;
				}
			}
		}
	}
	pointsAroundHighest.Sort([](const FVector2D& A, const FVector2D& B) {
		return A.Y != A.X ? A.Y > B.Y : A.X > B.X;
	});
	while (true) {
		float lengthWithoutTunel = 0.0;
		FVector2D lastVector = pointsAroundHighest[0];
		for (FVector2D vec : pointsAroundHighest)
		{
			if (vec.Y == lastVector.Y) continue;
			float heightDiffence = getMapHeightAtPoint(FVector2D(-57100 + 111 * vec.X, -57100 + 111 * vec.Y)) - getMapHeightAtPoint(FVector2D(-57100 + 111 * lastVector.X, -57100 + 111 * lastVector.Y));
			float length = sqrt(12321 + heightDiffence * heightDiffence);
			lengthWithoutTunel += length;
		}
		float lengthWithTunel = 0.0;
		FVector2D startPoint = FVector2D(pointsAroundHighest[0].X, pointsAroundHighest[0].Y);
		FVector2D endPoint = FVector2D(pointsAroundHighest[pointsAroundHighest.Num() - 1].X, pointsAroundHighest[pointsAroundHighest.Num() - 1].Y);
		float heightDiff = getMapHeightAtPoint(FVector2D(-57100 + 111 * startPoint.X, -57100 + 111 * startPoint.Y)) - getMapHeightAtPoint(FVector2D(-57100 + 111 * endPoint.X, -57100 + 111 * endPoint.Y));
		lengthWithTunel = sqrt((111 * abs(endPoint.Y - startPoint.Y)) * (111 * abs(endPoint.Y - startPoint.Y)) + heightDiff * heightDiff);

		//UE_LOG(LogTemp, Warning, TEXT("Heigh %f"), lengthWithoutTunel);
		//UE_LOG(LogTemp, Warning, TEXT("Heigh %f"), lengthWithTunel);

		if (lengthWithoutTunel < lengthWithTunel * 5 || pointsAroundHighest.Num() < 4)
		{
			//UE_LOG(LogTemp, Warning, TEXT("Heigh %f %f %d"), lengthWithoutTunel, lengthWithTunel * 5, pointsAroundHighest.Num());
			break;
		}
		else
		{
			pointsAroundHighest.RemoveAt(0);
			pointsAroundHighest.RemoveAt(pointsAroundHighest.Num() - 1);
		}
	}
}

void ATest3::calculateTurn()
{
	FVector2D directions[8]
	{
		{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
	};
	FVector firstTurn = turns[0];
	int firstDirection = firstTurn.Z;
	int directionChange = 0;
	int lastDirectionChange = 0;
	int lastDirection = firstDirection;
	FVector beginningOfTurn = FVector(firstTurn.X, firstTurn.Y, firstTurn.Z);
	TArray<TPair<FVector, FVector>> endPoints;
	for (int i = 1; i < turns.Num(); i++)
	{
		directionChange = lastDirection - turns[i].Z;
		if (lastDirectionChange != directionChange)
		{
			lastDirectionChange = directionChange;
			endPoints.Add(TPair<FVector, FVector>(FVector(beginningOfTurn.X, beginningOfTurn.Y, beginningOfTurn.Z), FVector(turns[i].X, turns[i].Y, turns[i].Z)));
			beginningOfTurn = FVector(turns[i - 1].X, turns[i - 1].Y, turns[i - 1].Z);
			i--;
		}
		lastDirection = turns[i].Z;
	}
	TPair<FVector2D, FVector2D> bestPair;
	float bestValue = 0;
	for (TPair<FVector, FVector> pair : endPoints)
	{
		float lengthWithoutTunel = 0.0;
		FVector2D startingPoint = FVector2D(pair.Key.X, pair.Key.Y);
		FVector2D currentPoisition = FVector2D(startingPoint.X, startingPoint.Y);
		FVector2D nextPoint = FVector2D(currentPoisition.X, currentPoisition.Y);
		TArray<FVector2D> poss;
		while (nextPoint.X != pair.Value.Y && nextPoint.Y != pair.Value.Y)
		{
			for (int i = 0; i < 8; i++)
			{
				nextPoint.Set(currentPoisition.X + directions[i].X, currentPoisition.Y + directions[i].Y);
				if (positions.Contains(nextPoint))
				{
					poss.Add(nextPoint);
					while (true)
					{
						FVector2D anotherPoint = FVector2D(nextPoint.X + directions[i].X, nextPoint.Y + directions[i].Y);
						if (positions.Contains(anotherPoint))
						{
							nextPoint = anotherPoint;
							poss.Add(nextPoint);
						}
						else
						{
							currentPoisition.Set(nextPoint.X, nextPoint.Y);
							i = 8;
							break;
						}
					}
				}
			}
		}
		FVector2D lastVector = poss[0];
		float lastHeight = getMapHeightAtPoint(FVector2D(-57100 + 111 * lastVector.X, -57100 + 111 * lastVector.Y));
		for (int j = 1; j < poss.Num(); j++)
		{
			FVector2D nextVector = poss[j];
			float nextHeight = getMapHeightAtPoint(FVector2D(-57100 + 111 * nextVector.X, -57100 + 111 * nextVector.Y));

			float heightDiffence = nextHeight - lastHeight;
			int move = 0;
			if (abs(nextVector.X - lastVector.X) == 1 && abs(nextVector.X - lastVector.X) == 1) move = 12321;
			else move = 24642;
			float length = sqrt(move + heightDiffence * heightDiffence);
			lengthWithoutTunel += length;
			
			lastVector = nextVector;
			lastHeight = nextHeight;
		}
		FVector2D s = FVector2D(pair.Key.X, pair.Key.Y);
		FVector2D e = FVector2D(pair.Value.X, pair.Value.Y);
		float heightDiff = getMapHeightAtPoint(FVector2D(-57100 + 111 * s.X, -57100 + 111 * s.Y)) - getMapHeightAtPoint(FVector2D(-57100 + 111 * e.X, -57100 + 111 * e.Y));
		float lengthWithTunel = sqrt((111 * abs(e.Y - s.Y)) * (111 * abs(e.Y - s.Y)) + heightDiff * heightDiff);
		float pathValue = lengthWithoutTunel - lengthWithTunel;
		if (pathValue > bestValue)
		{
			bestValue = pathValue;
			bestPair = TPair<FVector2D, FVector2D>(FVector2D(s.X, s.Y), FVector2D(e.X, e.Y));
		}
	}
	turnTunnel = getPointsBetween(bestPair.Key, bestPair.Value);
}

void ATest3::allDirectionalMovement()
{
	FVector2D directions[8]
	{
		{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
	};
	
	int32 startIndex = ((startPosition.Y * 1024) + startPosition.X);
	float min_value = FinalImage[startIndex];
	int32 nextIndex;
	for (int y = 0; y < 1024; y++)
	{
		for (int x = 0; x < 1024; x++)
		{
			int32 curPixelIndex = ((y * 1024) + x);
			if (FinalImage[curPixelIndex] - 2 < min_value)
			{

				int neighbours = 0;
				for (int i = 0; i < 8; i++)
				{
					nextIndex = (((y + directions[i].Y) * 1024) + (x + directions[i].X));
					if ((FinalImage[nextIndex] - 2 < min_value))
					{
						neighbours++;
					}
				}
				if (neighbours == 2)
				{
					minPos.Add(FVector2D(x, y));
				}	
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Heigh %d"), minPos.Num());
	minPos.Sort([](const FVector2D& A, const FVector2D& B) {
		return A.Y != A.X ? A.Y > B.Y : A.X > B.X;
	});
	for (int i = 1; i < minPos.Num(); i++)
	{
		FVector2D currentVector = minPos[i - 1];
		FVector2D nextVector = minPos[i];
		if (abs(currentVector.X - nextVector.X) > 1 || abs(currentVector.Y - nextVector.Y) > 1)
		{
			TArray<FVector2D> pts = getPointsBetween(currentVector, nextVector);
			for (FVector2D pt : pts)
			{
				allDirectionalPath.Add(pt);
			}
		}
		else
		{
			allDirectionalPath.Add(currentVector);
		}
	}
	if (!allDirectionalPath.Contains(endPosition))
	{
		allDirectionalPath.Add(endPosition);
	}
}


TArray<FVector2D> ATest3::getPointsBetween(FVector2D start, FVector2D end)
{
	TArray<FVector2D> result;

	int x1 = start.X;
	int y1 = start.Y;
	int x2 = end.X;
	int y2 = end.Y;
	
	int dx, dy, err, e2, sx, sy;

	dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
	dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
	err = (dx > dy ? dx : -dy) / 2;
	while (x1 != x2 || y1 != y2) {
		result.Add(FVector2D(x1, y1));
		e2 = err;
		if (e2 > -dx) {
			err -= dy;
			x1 += sx;
		}
		if (e2 < dy) {
			err += dx;
			y1 += sy;
		}
	}
	result.Add(FVector2D(x2, y2));
	
	return result;
}


void ATest3::generateCurvePoints(float* image, const int imageSize)
{
	int pos_x = startPosition.X; int pos_y = startPosition.Y;
	positions.Add(FVector2D(pos_x, pos_y));
	FVector2D directions[8]
	{
		{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
	};
	
	int32 curPixelIndex = ((pos_y * imageSize) + pos_x);
	float min_value = FLT_MAX;
	int direction = 0;
	for (int i = 0; i < 8; i++)
	{
		int32 otherPixelIndex = curPixelIndex + directions[i].Y * imageSize + directions[i].X;
		if (image[otherPixelIndex] < min_value)
		{
			min_value = image[otherPixelIndex];
			direction = i;
		}
	}
	pos_x += directions[direction].X;
	pos_y += directions[direction].Y;
	positions.Add(FVector2D(pos_x, pos_y));
	for (int tries = 0; tries < 10000; tries++)
	//while (pos_x != endPosition.X || pos_y != endPosition.Y)
	{
		min_value = FLT_MAX;
		curPixelIndex = ((pos_y * imageSize) + pos_x);
		int min_direction = direction;
		for (int i = -1; i < 2; i++)
		{
			int dir = (direction + i) % 8;
			if (dir == -1) dir = 7;
			int32 otherPixelIndex = curPixelIndex + directions[dir].Y * imageSize + directions[dir].X;
			if (i == 0)
			{
				if (image[otherPixelIndex] < min_value)
				{
					min_value = image[otherPixelIndex];
					min_direction = dir;
				}
			}
			else
			{
				if (image[otherPixelIndex] + turnCoefficient < min_value)
				{
					min_value = image[otherPixelIndex] + turnCoefficient;
					min_direction = dir;
				}
			}
		}
		if (min_direction != direction)
		{
			turns.Add(FVector(pos_x, pos_y, min_direction));
		}
		direction = min_direction;
		pos_x += directions[direction].X;
		pos_y += directions[direction].Y;
		positions.Add(FVector2D(pos_x, pos_y));
		if (pos_x == endPosition.X && pos_y == endPosition.Y)
		{
			 break;
		}
	}
}

void ATest3::destroyOldSpline(bool destroyMesh)
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

FVector ATest3::getSplineLocationAtPosition(FVector2D position)
{
	return FVector(position.X, position.Y, this->getMapHeightAtPoint(position));
}

TArray<FVector> ATest3::createPointsAtSpacing()
{
	TArray<FVector> locationsToAdd;
	float currentDistance = 0;
	float totalDistance = SplineComponent->GetSplineLength();
	int numberOfPoints = totalDistance / pointPrecision;
	for (int index = 0; index <= numberOfPoints; index++)
	{
		FVector oldPosition = SplineComponent->GetLocationAtDistanceAlongSpline(currentDistance, ESplineCoordinateSpace::Local);
		locationsToAdd.Add(this->getSplineLocationAtPosition(FVector2D(oldPosition.X, oldPosition.Y)));
		currentDistance += pointPrecision;
	}
	FVector endPoint = SplineComponent->GetLocationAtSplinePoint(SplineComponent->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::Local);
	locationsToAdd.Add(this->getSplineLocationAtPosition(FVector2D(endPoint.X, endPoint.Y)));
	return locationsToAdd;
}

FVector ATest3::getUpVectorAtPoint(FVector2D point)
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

void ATest3::initializeCurve()
{
	SplineMeshComponents.Empty();
	for (int index = 1; index < positions.Num(); index++)
	{
		SplineComponent->AddSplinePoint(FVector(-57100 + 111 * positions[index].X, -57100 + 111 * positions[index].Y, 10), ESplineCoordinateSpace::Local);
	}
	TArray<FVector> locationsToAdd = this->createPointsAtSpacing();
	this->destroyOldSpline(false);
	for (int index = 0; index < locationsToAdd.Num(); index++)
	{
		SplineComponent->AddSplinePoint(FVector(locationsToAdd[index].X, locationsToAdd[index].Y, locationsToAdd[index].Z), ESplineCoordinateSpace::Local, true);
	}
}

void ATest3::drawSpline()
{
	if (!Mesh)
	{
		return;
	}
	int spacing = 500;
	int line_width = 500;
	this->DestroyConstructedComponents();
	this->destroyOldSpline(true);
	this->initializeCurve();
	float curveLength = SplineComponent->GetSplineLength();
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
void ATest3::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Warning, TEXT("Time %f"), DeltaTime);
	if (animation)
	{
		FVector2D pos = positions[currentPlayerIndex];

		if (pos.X != endPosition.X && pos.Y != endPosition.Y)
		{
			
			FVector pos3 = FVector(-57100 + 111 * pos.X, -57100 + 111 * pos.Y, 200 + getMapHeightAtPoint(FVector2D(-57100 + 111 * pos.X, -57100 + 111 * pos.Y)));

			//UE_LOG(LogTemp, Warning, TEXT("Time %f"), DeltaTime);

			//FVector lastPos = player->GetActorLocation();

			//FVector diff = FVector(lastPos.X - pos3.X, lastPos.Y - pos3.Y, lastPos.Z - pos3.Z);
			
			player->SetActorLocation(pos3);

			//player->SetActorRotation(diff.Rotation());

			currentPlayerIndex++;
		}
		else
		{
			animation = false;
		}
	}
	
}

