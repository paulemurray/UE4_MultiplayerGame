// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class CSP_MultiplayerGameTarget : TargetRules
{
	public CSP_MultiplayerGameTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.Add("CSP_MultiplayerGame");
	}
}
