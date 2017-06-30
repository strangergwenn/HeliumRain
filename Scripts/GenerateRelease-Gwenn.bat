@echo off

set OutputDir=D:\HR-Builds
set ProjectDir=D:\HeliumRain
set EngineDir=D:\Dock\UE4_Packaged\UE_4.16

set VersionName=beta-300617
if not "%1"=="" (
	set VersionName=%1
)

call GenerateRelease.bat %VersionName% %OutputDir% %ProjectDir% %EngineDir%
