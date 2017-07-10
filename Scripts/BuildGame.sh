#!/bin/bash

# =======================================================
# Settings
# =======================================================

Platform=Linux
BuildMode=Development


# =======================================================
# Release variables
# =======================================================

VersionName=$1
InputProject=$2
OutputDir=$3
UAT=$4


# =======================================================
# Run UAT
# =======================================================

#$UAT -ScriptsForProject=$InputProject BuildCookRun -nocompile -nocompileeditor -installed -nop4 -project=$InputProject -cook -stage -archive -archivedirectory=$OutputDir -package -clientconfig=$BuildMode -ue4exe=UE4Editor -clean -pak -compressed -prereqs -distribution -nodebuginfo -createreleaseversion=$VersionName -targetplatform=$Platform -build -utf8output -CookCultures=en-US+fr-FR

#$UAT -ScriptsForProject=$InputProject BuildCookRun -nocompile -nocompileeditor -installed -nop4 -project=$InputProject -cook -stage -archive -archivedirectory=$OutputDir -package -LinuxNoEditor -clientconfig=$BuildMode -ue4exe=UE4Editor -clean -pak -compressed -prereqs -distribution -nodebuginfo -createreleaseversion=$VersionName -targetplatform=$Platform -build -utf8output -CookCultures=en-US+fr-FR


#$UAT -ScriptsForProject=$InputProject nocompileeditor     -build -utf8output -CookCultures=en-US+fr-FR



# full
## $UAT BuildCookRun -nocompile -nop4 -project=$InputProject -cook -compressed     -prereqs -distribution -nodebuginfo -createreleaseversion=$VersionName -installed -build -nocompileeditor     -allmaps -stage -archive -archivedirectory=$OutputDir -package -LinuxNoEditor -clientconfig=$BuildMode -ue4exe=UE4Editor -clean -pak -targetplatform=Linux -utf8output -CookCultures=en-US+fr-FR

# cooking ok
#$UAT  -ScriptsForProject=$InputProject BuildCookRun -nocompile -nop4 -project=$InputProject -cook -compressed     -prereqs -distribution -nodebuginfo -createreleaseversion=$VersionName -installed  -nocompileeditor     -allmaps -stage -archive -archivedirectory=$OutputDir -package -LinuxNoEditor -clientconfig=$BuildMode -ue4exe=UE4Editor -clean -pak -targetplatform=Linux -utf8output -CookCultures=en-US+fr-FR

# build ok, cooked ,  archive ok
#$UAT  BuildCookRun -nocompile -nop4 -project=$InputProject -cook -compressed -allmaps -build -stage -archive -archivedirectory=$OutputDir -package -LinuxNoEditor -clientconfig=$BuildMode -ue4exe=UE4Editor -clean -pak -targetplatform=Linux -utf8output -CookCultures=en-US+fr-FR


$UAT  BuildCookRun -nocompile -nop4 -project=$InputProject -cook -compressed -allmaps -build          -distribution -nodebuginfo -createreleaseversion=$VersionName            -stage -archive -archivedirectory=$OutputDir -package -LinuxNoEditor -clientconfig=$BuildMode -ue4exe=UE4Editor -clean -pak -targetplatform=Linux -utf8output -CookCultures=en-US+fr-FR


# working
#$UAT BuildCookRun -nocompile -nop4 -project=$InputProject -cook -compressed -allmaps -stage -archive -archivedirectory=$OutputDir -package -LinuxNoEditor -clientconfig=$BuildMode -ue4exe=UE4Editor -clean -pak -targetplatform=Linux -utf8output -CookCultures=en-US+fr-FR


#$UAT -ScriptsForProject=$InputProject BuildCookRun -nocompile -nop4 -project=$InputProject -cook -stage -archive -archivedirectory=$OutputDir -package -clientconfig=$BuildMode -ue4exe=UE4Editor -clean -pak -compressed -prereqs -distribution -nodebuginfo -createreleaseversion=$VersionName -targetplatform=$Platform -build -utf8output
#Engine/Build/BatchFiles/RunUAT.sh     BuildCookRun -nocompile                             -nop4 -project="/absolute/path/to/your/MyProject/MyProject.uproject" -cook -compressed 
#-allmaps -stage -archive -archivedirectory="/absolute/path/to/output/folder/" -package -LinuxNoEditor -clientconfig=Development -ue4exe=UE4Editor -clean 
#-pak -targetplatform=Linux -utf8output

#cd /home/fred/store/workspace/4.11/UnrealEngine
#Engine/Build/BatchFiles/RunUAT.sh BuildCookRun -nocompile -installed -nop4 -project="$InputProject" -cook -compressed -allmaps -stage -archive -archivedirectory="$OutputDir" -package -LinuxNoEditor -clientconfig=Development -ue4exe=UE4Editor -clean -pak -targetplatform=Linux -utf8output
#cd -
