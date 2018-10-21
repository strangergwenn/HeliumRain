#!/usr/bin/env python
import os
import json

# Get Itch settings
itchData = open("itch.json")
itchConfig = json.load(itchData)
itchUser = itchConfig["user"]
itchProject = itchConfig["project"]
itchBranches = itchConfig["branches"]
itchDirectories = itchConfig["directories"]

# Upload all platforms (Itch)
itchBranchIndex = 0
for branch in itchBranches:

	# Generate full command line
	commandLine = "butler push "
	commandLine += itchDirectories[itchBranchIndex] + " "
	commandLine += itchUser + "/" + itchProject + ":" + branch

	# Call
	os.system(commandLine)
	itchBranchIndex += 1
