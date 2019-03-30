// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class CSP_MultiplayerGameEditorTarget : TargetRules
{
	public CSP_MultiplayerGameEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		ExtraModuleNames.Add("CSP_MultiplayerGame");
	}
}
