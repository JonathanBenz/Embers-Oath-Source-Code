// Copyright (c) 2025, Fools With Blades. All rights reserved.
// Contributors: Jonathan Benz

#include "AI/C_SmoothFocusAIController.h"
#include "Kismet/KismetMathLibrary.h"

void AC_SmoothFocusAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
	Super::UpdateControlRotation(DeltaTime, false);

	// Smooth and change the pawn rotation
	if (bUpdatePawn)
	{
		APawn* const MyPawn = GetPawn();
		const FRotator CurrentPawnRotation = MyPawn->GetActorRotation();

		// Calculate smoothed rotation
		SmoothTargetRotation = UKismetMathLibrary::RInterpTo_Constant(MyPawn->GetActorRotation(), ControlRotation, DeltaTime, SmoothFocusInterpSpeed);
		//C heck if we need to change
		if (CurrentPawnRotation.Equals(SmoothTargetRotation, 1e-3f) == false)
		{
			// Change rotation using the Smooth Target Rotation
			MyPawn->FaceRotation(SmoothTargetRotation, DeltaTime);
		}
	}
}