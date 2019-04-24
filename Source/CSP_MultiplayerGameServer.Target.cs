using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]

public class CSP_MultiplayerGameServerTarget : TargetRules
{
    public CSP_MultiplayerGameServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        ExtraModuleNames.Add("CSP_MultiplayerGame");
    }
} 