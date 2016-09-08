
@echo off

REM =======================================================
REM Release variables
REM =======================================================

SET VersionName=%1
SET InputProject=%2
SET OutputDir=%3
set UAT=%4

REM =======================================================
REM Additional settings
REM =======================================================
set Platform=Win64
set BuildMode=Development
set SessioName=Release
set SessionOwner=AutoBuild
Set MapList=Anomaly+Colossus+Space+Spire

REM =======================================================
REM Run the damn thing
REM =======================================================

%UAT% -ScriptsForProject=%InputProject% BuildCookRun -project=%InputProject% -noP4 -clientconfig=%BuildMode% -serverconfig=%BuildMode% -nocompile -nocompileeditor -installed -ue4exe=UE4Editor-Cmd.exe -utf8output -platform=%Platform% -targetplatform=%Platform% -build -cook -map=%MapList% -unversionedcookedcontent -pak -createreleaseversion=%VersionName% -distribution -compressed -stage -package -stagingdirectory=%OutputDir% -cmdline=" -Messaging" -addcmdline="-SessionOwner='%SessionOwner%' -SessionName='%SessioName%'"
