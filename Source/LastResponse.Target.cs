using UnrealBuildTool;

public class LastResponseTarget : TargetRules
{
	public LastResponseTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("LastResponse");
	}
}
