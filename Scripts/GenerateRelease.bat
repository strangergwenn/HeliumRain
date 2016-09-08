@echo off

rem =======================================================
rem Parameters
rem =======================================================

set VersionName=%1
set OutputDir=%2
set ProjectDir=%3
set EngineDir=%4


rem =======================================================
rem Intermediary files and directories
rem =======================================================

set InputProject=%ProjectDir%\HeliumRain.uproject
set ReleaseNotes=%ProjectDir%\Scripts\ReleaseNotes.xml
set ManifestTool=%ProjectDir%\Scripts\UnrealManifest.exe
set UAT=%EngineDir%\Engine\Build\BatchFiles\RunUAT.bat


rem =======================================================
rem Build process
rem =======================================================

call BuildGame.bat %VersionName% %InputProject% %OutputDir% %UAT%

pushd %OutputDir%\WindowsNoEditor

del /s *.txt
%ManifestTool%
copy %ReleaseNotes%

popd
