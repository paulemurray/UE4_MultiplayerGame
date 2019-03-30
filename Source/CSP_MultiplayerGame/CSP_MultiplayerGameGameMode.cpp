// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "CSP_MultiplayerGameGameMode.h"
#include "CSP_MultiplayerGameHUD.h"
#include "CSP_MultiplayerGameCharacter.h"
#include "UObject/ConstructorHelpers.h"

ACSP_MultiplayerGameGameMode::ACSP_MultiplayerGameGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = ACSP_MultiplayerGameHUD::StaticClass();
}
