@echo off

rem =======================================================
rem Settings
rem =======================================================

set Platform=Win64
set BuildMode=Shipping
set SessioName=Release
set SessionOwner=AutoBuild
Set MapList=Anomaly+Colossus+Space+Spire


rem =======================================================
rem Release variables
rem =======================================================

set VersionName=%1
set InputProject=%2
set OutputDir=%3
set UAT=%4


rem =======================================================
rem Run UAT
rem =======================================================

%UAT% -ScriptsForProject=%InputProject% BuildCookRun -project=%InputProject% -noP4 -clientconfig=%BuildMode% -serverconfig=%BuildMode% -nocompile -nocompileeditor -installed -ue4exe=UE4Editor-Cmd.exe -utf8output -platform=%Platform% -targetplatform=%Platform% -build -cook -map=%MapList% -unversionedcookedcontent -pak -prereqs -createreleaseversion=%VersionName% -distribution -nodebuginfo -compressed -stage -package -stagingdirectory=%OutputDir% -cmdline=" -Messaging" -addcmdline="-SessionOwner='%SessionOwner%' -SessionName='%SessioName%'"
