# Helium Rain source code

Helium Rain is a realistic space opera for PC, now available on Steam.

 - [Website](http://helium-rain.com)
 - [Store page](https://store.steampowered.com/app/681330)

![Game screenshot](http://helium-rain.com/gallery_data/blueheart.jpg)

## About the game

Helium Rain is a single-player space sim that places you at the helm of a spacefaring company. Exploration, trading, station-building, piracy are all options. Helium Rain relies on both spaceflight and strategy gameplay, mixed together in a creative way. Destroying a freighter has a direct impact on the economy, while declaring war will make your environment more hostile.

 - Realistic economy model with supply and demand
 - Strategy gameplay with procedural quests, world exploration, technology upgrades
 - 12 playable ships with weapon and engine upgrades
 - Fast-paced combat with a Newtonian flight model
 - Localized damage model for spacecrafts
 - Quick-play skirmish mode

![Game screenshot](http://helium-rain.com/gallery_data/orbits.jpg)

## Building Helium Rain from source

We provide these sources for our customers, and as a reference for Unreal Engine developers. **You won't be able to run the game from this repository alone**, as the game contents are not included. Building from source is only useful if you want to replace the game executable with your modifications.

Building and modifying the source code for Helium Rain requires a few steps: getting the Unreal Engine 4 and other required tools, and building the game. We recommend you use the **release** branch of the game, but you can also keep the default **master** branch if you want to keep up with our changes.

### Required dependencies
You will need the following tools to build Helium Rain from the sources:

* Helium Rain uses UE4 as a game engine. You can get it for free at [unrealengine.com](http://unrealengine.com). You will need to sign up and download the Epic Games launcher. In the launcher library for Unreal Engine, install version 4.20.
* [Visual Studio Community 2017](https://www.visualstudio.com/downloads/) will be used to build the sources. Don't forget to select the C++ development environment, since this is optional.
* The [Windows 8.1 SDK](https://developer.microsoft.com/en-us/windows/downloads/windows-8-1-sdk) is required for Unreal Engine 4.
* The [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812) is required for the joystick plugin.
* [CMake](https://cmake.org/download) is required for the joystick plugin. When prompted to add to the system PATH, please do it.
* [TortoiseHg](https://tortoisehg.bitbucket.io/) is required for the joystick plugin.

### Build process
We will now build the Helium Rain game executable. Follow these steps.

* Open a Windows console (Windows + R ; "cmd" ; Enter).
* Navigate to the *Plugins\JoystickPlugin\ThirdParty\SDL2 folder* in the Helium Rain archive.
* Run setup.bat and wait for it to complete without errors.
* Run build.bat and wait for it to complete without errors.
* In the Windows explorer, right-click HeliumRain.uproject and pick "Generate Visual Studio Project Files".
* A HeliumRain.sln file will appear - double-click it to open Visual Studio.
* Select the "Shipping" build type.
* You can now build Helium Rain by hitting F7 or using the Build menu. This should take from 5 to 10 minutes.

The resulting binary will be generated as *Binaries\Win64\HeliumRain-Win64-Shipping.exe* and can replace the equivalent file in your existing game folder.
