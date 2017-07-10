#!/bin/bash
# =======================================================
# Parameters
# =======================================================

VersionName=$1
OutputDir=$2
ProjectDir=$3
EngineDir=$4


# =======================================================
# Intermediary files and directories
# =======================================================

InputProject=$ProjectDir/HeliumRain.uproject
ReleaseNotes=$ProjectDir/Scripts/ReleaseNotes.xml
ManifestTool=$ProjectDir/Scripts/UnrealManifest.exe
UAT=$EngineDir/Engine/Build/BatchFiles/RunUAT.sh


# =======================================================
# Build process
# =======================================================

./BuildGame.sh $VersionName $InputProject $OutputDir $UAT
	
cd $OutputDir/LinuxNoEditor

find -name "*.txt | xargs rm
#%ManifestTool%
cp $ReleaseNotes .

cd -
