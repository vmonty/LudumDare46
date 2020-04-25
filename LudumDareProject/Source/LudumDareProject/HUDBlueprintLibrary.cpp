// Fill out your copyright notice in the Description page of Project Settings.

#include "MOSExample.h"
#include "HUDBlueprintLibrary.h"

void UHUDBlueprintLibrary::FindScreenEdgeLocationForWorldLocation(UObject* WorldContextObject, const FVector& InLocation, const float EdgePercent, FVector2D& OutScreenPosition, float& OutRotationAngleDegrees, bool &bIsOnScreen)
{
	bIsOnScreen = false;
	OutRotationAngleDegrees = 0.f;

	if (!GEngine) return;

	const FVector2D ViewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
	const FVector2D  ViewportCenter = FVector2D(ViewportSize.X / 2, ViewportSize.Y / 2);

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject);

	if (!World) return;

	APlayerController* PlayerController = (WorldContextObject ? UGameplayStatics::GetPlayerController(WorldContextObject, 0) : NULL);

	if (!PlayerController) return;

	ACharacter* PlayerCharacter = Cast<ACharacter>(PlayerController->GetPawn());

	if (!PlayerCharacter) return;

	FVector Forward = PlayerCharacter->GetActorForwardVector();
	FVector Offset = (InLocation - PlayerCharacter->GetActorLocation()).GetSafeNormal();

	float DotProduct = FVector::DotProduct(Forward, Offset);
	bool bLocationIsBehindCamera = (DotProduct < 0);

	if (bLocationIsBehindCamera)
	{
		// For behind the camera situation, we cheat a little to put the
		// marker at the bottom of the screen so that it moves smoothly
		// as you turn around. Could stand some refinement, but results
		// are decent enough for most purposes.

		FVector DiffVector = InLocation - PlayerCharacter->GetActorLocation();
		FVector Inverted = DiffVector * -1.f;
		FVector NewInLocation = PlayerCharacter->GetActorLocation() * Inverted;

		NewInLocation.Z -= 5000;

		PlayerController->ProjectWorldLocationToScreen(NewInLocation, OutScreenPosition);
		OutScreenPosition.Y = (EdgePercent * ViewportCenter.X) * 2.f;
		OutScreenPosition.X = -ViewportCenter.X - OutScreenPosition.X;
	}

	PlayerController->ProjectWorldLocationToScreen(InLocation, OutScreenPosition);//*ScreenPosition);

	// Check to see if it's on screen. If it is, ProjectWorldLocationToScreen is all we need, return it.	
	if (OutScreenPosition.X >= 0.f && OutScreenPosition.X <= ViewportSize.X
		&& OutScreenPosition.Y >= 0.f && OutScreenPosition.Y <= ViewportSize.Y)
	{

		bIsOnScreen = true;
		return;
	}

	OutScreenPosition -= ViewportCenter;

	float AngleRadians = FMath::Atan2(OutScreenPosition.Y, OutScreenPosition.X);
	AngleRadians -= FMath::DegreesToRadians(90.f);

	OutRotationAngleDegrees = FMath::RadiansToDegrees(AngleRadians) + 180.f;

	float Cos = cosf(AngleRadians);
	float Sin = -sinf(AngleRadians);

	OutScreenPosition = FVector2D(ViewportCenter.X + (Sin * 180.f), ViewportCenter.Y + Cos * 180.f);

	float m = Cos / Sin;

	FVector2D ScreenBounds = ViewportCenter * EdgePercent;

	if (Cos > 0)
	{
		OutScreenPosition = FVector2D(ScreenBounds.Y / m, ScreenBounds.Y);
	}
	else
	{
		OutScreenPosition = FVector2D(-ScreenBounds.Y / m, -ScreenBounds.Y);
	}

	if (OutScreenPosition.X > ScreenBounds.X)
	{
		OutScreenPosition = FVector2D(ScreenBounds.X, ScreenBounds.X*m);
	}
	else if (OutScreenPosition.X < -ScreenBounds.X)
	{
		OutScreenPosition = FVector2D(-ScreenBounds.X, -ScreenBounds.X*m);
	}

	OutScreenPosition += ViewportCenter;

}