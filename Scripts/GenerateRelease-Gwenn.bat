@echo off

SET VersionName=%1
SET InputProject=D:\HeliumRain\HeliumRain.uproject
SET OutputDir=D:\HR-Builds\Build
set UAT=D:\Dock\UE4_Packaged\4.13\Engine\Build\BatchFiles\RunUAT.bat

GenerateRelease.bat %VersionName% %InputProject% %OutputDir% %UAT%
