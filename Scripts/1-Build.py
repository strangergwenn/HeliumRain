#!/usr/bin/env python
import os
import sys
import json
import shutil
import subprocess

# Get project settings
projectData = open("project.json")
projectConfig = json.load(projectData)
buildCultures = projectConfig["cultures"]
buildConfiguration = projectConfig["configuration"]
projectData.close()

# Get system settings
systemData = open("system.json")
systemConfig = json.load(systemData)
buildPlatforms = systemConfig["platforms"]
inputDir = systemConfig["inputDir"]
outputDir = systemConfig["outputDir"]
engineDir = systemConfig["engineDir"]
engineInstalled = systemConfig["engineInstalled"]
systemData.close()

# Platform-dependent names
if sys.platform.startswith('linux'):
	scriptExt = ".sh"
	engineExecutable = "UE4Editor"
	noCompileEditorOption = " -LinuxNoEditor"
else:
	scriptExt = ".bat"
	engineExecutable = "UE4Editor-Cmd.exe"
	noCompileEditorOption = " -nocompileeditor"

# Installed vs built engine
if engineInstalled:
	installedOption = " -installed"
	cleanOption = " -clean"
else:
	installedOption = ""
	cleanOption = ""
	
# Generate paths
inputProject = os.path.join(inputDir, "HeliumRain.uproject")
buildTool = os.path.join(engineDir, "Engine", "Build", "BatchFiles", "RunUAT" + scriptExt)

# Generate version tag
gitCommand = ['git', 'describe']
buildVersion = subprocess.check_output(gitCommand).decode("utf-8")
buildVersion = buildVersion.replace("\n", "");

# Build each platform
for platform in buildPlatforms:

	# Generate command line
	commandLine = buildTool
	commandLine += " BuildCookRun -project=" + inputProject + " -nocompile" + noCompileEditorOption + installedOption
	commandLine += " -nop4 -clientconfig=" + buildConfiguration
	commandLine += " -cook -allmaps -stage -archive -archivedirectory=" + outputDir
	commandLine += " -package -ue4exe=" + engineExecutable
	commandLine += " -build -targetplatform=" + platform + cleanOption
	commandLine += " -pak -prereqs -distribution -createreleaseversion=" + buildVersion
	commandLine += " -utf8output -CookCultures=" + buildCultures

	# Call
	os.system(commandLine)
	
	# Copy Boiler files and other tools
	if projectConfig["modding"]:
		if platform == 'Linux':
			buildOutputDir = outputDir + "/LinuxNoEditor"
			shutil.copyfile("../HeliumRainLauncher", buildOutputDir + "/HeliumRainLauncher")
			shutil.copyfile("../HeliumRainLauncher.sh", buildOutputDir + "/HeliumRainLauncher.sh")
			shutil.copyfile("../libsteam_api.so", buildOutputDir + "/libsteam_api.so")
			shutil.copyfile("../steam_appid.txt", buildOutputDir + "/steam_appid.txt")
			shutil.copytree("../Icons", buildOutputDir + "/Icons")
		else:
			buildOutputDir = outputDir + "/WindowsNoEditor"
			shutil.copyfile("../HeliumRainLauncher.exe", buildOutputDir + "/HeliumRainLauncher.exe")
			shutil.copyfile("../steam_api64.dll", buildOutputDir + "/steam_api64.dll")
			shutil.copyfile("../steam_appid.txt", buildOutputDir + "/steam_appid.txt")
