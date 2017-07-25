#!/usr/bin/env python
import os
import json
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
buildPlatform = systemConfig["platform"]
inputDir = systemConfig["inputDir"]
outputDir = systemConfig["outputDir"]
engineDir = systemConfig["engineDir"]
engineInstalled = systemConfig["engineInstalled"]
systemData.close()

# Platform-dependent names
if buildPlatform == "Linux":
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

command = ['git', 'describe']
buildVersion = subprocess.check_output(command).decode("utf-8")
buildVersion = buildVersion.replace("\n", "");

# Generate command line
commandLine = buildTool
commandLine += " BuildCookRun -project=" + inputProject + " -nocompile" + noCompileEditorOption + installedOption
commandLine += " -nop4 -clientconfig=" + buildConfiguration
commandLine += " -cook -allmaps -stage -archive -archivedirectory=" + outputDir
commandLine += " -package -ue4exe=" + engineExecutable
commandLine += " -build -targetplatform=" + buildPlatform + cleanOption
commandLine += " -pak -prereqs -distribution -createreleaseversion=" + buildVersion
commandLine += " -utf8output -CookCultures=" + buildCultures

# Call
os.system(commandLine)
