using UnrealBuildTool;

public class LastResponseEditorTarget : TargetRules
{
	public LastResponseEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("LastResponse");
	}
}
