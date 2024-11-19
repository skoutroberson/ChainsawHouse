// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ChainsawHouseEditorTarget : TargetRules
{
	public ChainsawHouseEditorTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;

		ExtraModuleNames.AddRange( new string[] { "ChainsawHouse" } );
	}
}
