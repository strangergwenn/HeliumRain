
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
buildTool = os.path.join(engineDir, "Engine", "Build", "BatchFiles", "RunUAT.bat")

# Generate full command line
commandLine = buildTool
commandLine += " BuildCookRun -project=" + inputProject + " -nocompile -nocompileeditor -installed -nop4 -clientconfig=" + buildConfiguration
commandLine += " -cook -allmaps -stage -archive -archivedirectory=" + outputDir
commandLine += " -package -ue4exe=UE4Editor-Cmd.exe -build -clean -pak -prereqs -distribution -nodebuginfo -createreleaseversion=" + buildVersion
commandLine += " -utf8output -CookCultures=" + buildCultures

# Call
print(commandLine)
os.system(commandLine)
