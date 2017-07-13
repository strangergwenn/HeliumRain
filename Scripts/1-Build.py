#!/usr/bin/env python
import os
import json

# Get project settings
projectData = open("project.json")
projectConfig = json.load(projectData)
buildVersion = projectConfig["version"]
buildCultures = projectConfig["cultures"]
buildConfiguration = projectConfig["configuration"]
projectData.close()

# Get system settings
systemData = open("system.json")
systemConfig = json.load(systemData)
buildPlatform = systemConfig["platform"]
inputDir = systemConfig["inputDir"]
outputDir = systemConfig["outputDir"]
engineDir = systemConfig["engineDir"]
systemData.close()

# Generate paths
inputProject = os.path.join(inputDir, "HeliumRain.uproject")
if buildPlatform == "Linux":
	buildTool = os.path.join(engineDir, "Engine", "Build", "BatchFiles", "RunUAT.sh")
else:
	buildTool = os.path.join(engineDir, "Engine", "Build", "BatchFiles", "RunUAT.bat")

# Build
commandLine = buildTool
commandLine += " BuildCookRun -project=" + inputProject + " -nocompile"
if buildPlatform == "Linux":
	commandLine += " -LinuxNoEditor"
else:
	commandLine += " -nocompileeditor -installed"
commandLine += "-nop4 -clientconfig=" + buildConfiguration

# Cook
commandLine += " -cook -allmaps -stage -archive -archivedirectory=" + outputDir

# Package
commandLine += " -package"
if buildPlatform == "Linux":
	commandLine += " -ue4exe=UE4Editor -targetplatform=Linux"
else:
	commandLine += " -ue4exe=UE4Editor-Cmd.exe"
commandLine += " -build"
if buildPlatform == "Windows":
	commandLine += " -clean"
commandLine += " -pak -prereqs -distribution -nodebuginfo -createreleaseversion=" + buildVersion
commandLine += " -utf8output -CookCultures=" + buildCultures

# Call
os.system(commandLine)
