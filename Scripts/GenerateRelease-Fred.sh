#!/bin/bash

OutputDir=/home/fred/store/HR-Builds2
ProjectDir=/home/fred/store/workspace/4.11/HeliumRain
EngineDir=/home/fred/store/workspace/4.11/UnrealEngine

VersionName=beta-100717

if [ $# -eq 1 ]
	then
    VersionName=$1
fi

./GenerateRelease.sh $VersionName $OutputDir $ProjectDir $EngineDir
