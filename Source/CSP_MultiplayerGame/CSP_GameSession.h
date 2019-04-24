// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/OnlineSession.h"
#include "CSP_GameSession.generated.h"

/**
 * 
 */
UCLASS()
class CSP_MULTIPLAYERGAME_API ACSP_GameSession : public AGameSession
{
	GENERATED_BODY()

protected:
	virtual void RegisterServer() override;
};
