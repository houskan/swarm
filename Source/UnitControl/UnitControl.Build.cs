// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class UnitControl : ModuleRules
{
	public UnitControl(TargetInfo Target)
	{
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
        
        //PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\gurobi81.lib");
        //PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\gurobi_c++md2017.lib");
        //PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\gurobi_c++mdd2017.lib");
        /*PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\amplsolv.lib");
         * 
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libBcp.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libBonCouenne.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libbonmin.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libbonminampl.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libCbc.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libCbcSolver.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libCgl.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libClp.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libCoinUtils.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libCouenne.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libCouenneReadnl.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libDylp.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libipopt.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libOS.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libOsi.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libOsiCbc.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libOsiClp.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libOsiDylp.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libOsiSym.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libOsiVol.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libSym.lib");
        PublicAdditionalLibraries.Add(@"D:\Unreal Projects\UnitControl\Binaries\Win64\libVol.lib");*/

        PublicIncludePaths.AddRange(new string[] { "D:/gurobi/win64/include/" });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
