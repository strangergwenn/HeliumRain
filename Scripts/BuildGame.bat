@echo off

rem =======================================================
rem Settings
rem =======================================================

set Platform=Win64
set BuildMode=Development


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

%UAT% -ScriptsForProject=%InputProject% BuildCookRun -nocompile -nocompileeditor -installed -nop4 -project=%InputProject% -cook -stage -archive -archivedirectory=%OutputDir% -package -clientconfig=%BuildMode% -ue4exe=UE4Editor-Cmd.exe -clean -pak -compressed -prereqs -distribution -nodebuginfo -createreleaseversion=%VersionName% -targetplatform=%Platform% -build -utf8output -CookCultures=en-US+fr-FR
