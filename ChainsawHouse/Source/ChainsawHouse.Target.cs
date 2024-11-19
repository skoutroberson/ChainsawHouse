// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ChainsawHouseTarget : TargetRules
{
    public ChainsawHouseTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;

        ExtraModuleNames.AddRange(new string[] { "ChainsawHouse" });

    }
}