@echo off

set OutputDir=D:\HR-Builds
set ProjectDir=D:\HeliumRain
set EngineDir=D:\Dock\UE4

set VersionName=dev
if not "%1"=="" (
	set VersionName=%1
)

call GenerateRelease.bat %VersionName% %OutputDir% %ProjectDir% %EngineDir%
