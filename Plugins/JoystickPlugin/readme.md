# UEJoystickPlugin
This plugin uses SDL2 functions to get input instead of DirectInput.

Thank you Ikarus76 and samiljan for the good working basis. My implementation is a bit hacky, but I did not have much exercise and UE 4 is not easy.
I was free and did everything so adjusted as I need it for my lab projects. 

Status:
Sam Persson did code review, polishing and adding hotplug functionality. 

Links for Software needed to compile the plugin:

Windows:

* [Microsoft DirectX SDK Jun2010](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
* [Windows 8.1 SDK](https://msdn.microsoft.com/de-de/windows/desktop/bg162891.aspx#)
* [Windows 10 SDK (If using Windows 10)](https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk)


# How to use it

## Windows:
Compiling SDL2 with Visual C++ 2015 is a painful process. Compiling with the Dynamic or Static library will do. But I am not able get the static version linked to my plugin at the moment, so for now I am using the dynamically linked (DLL) version.

1. Clone the repository branch 4.11 for UE4 version 4.11 (Or whichever is applicable)
2. If the binary versions in repo are older, please consult the following steps. I put compiled versions for 4.10.1 in the repository.
	1. Go to Plugins/JoystickPlugin/ThirdParty/SDL2
	2. If you are doing this for the first time, run setup.bat. This will download the latest Mercurial branch source of SDL2.
	3. Run build.bat to build the latest version of SDL2. This batch-files will copy the binaries to the bin directory of the plugin.
	4. If you have installed CMake, Visual Studio 2013 or 2015, DirectX SDK Jun2010, the Windows 8.1/10 SDK and have built the SDL2 code, you should get SDL2 files in the SDL2/Lib directory. 
3. You should have an UE4 Project (your project) with C++ Sources. If you do not have C++ sources, then create a Dummy Class. This will create an Visual Studio Project (to recreate: File -> Refresh Visual Studio Project). Once you have created a Class, the Visual Studio Editor comes up, the Editor initiates compiling in background. Now you can close VS and UE4 Editor.
4. Copy the Plugins-Directory you checked out to your project. The Plugins folder should be in the root of the project. Refresh the Visual Studio Project files: File -> Refresh Visual Studio Project.
5. Open the UE4 Project and be sure you have activated the JoystickPlugin.

Now you can map Inputs to Joystick devices in Project Settings. (Engine->Input)

Here is a Test Project using the Third-Person Template: [Download](https://w-hs.sciebo.de/index.php/s/148QVopCDdHwhLQ)
Here is an minimal demo project: [Download](https://w-hs.sciebo.de/index.php/s/qajqJPsk1JGhFFM)

-----------------------------------------------------------------------------------------------------------
## Linux (Ubuntu 14.04 LTS): 
*(TODO: test on fresh install systems (maybe I forgot something))*

### Compile SDL2 as static library from source:

1. Go to Engine/Source/ThirdParty/SDL2
2. Call build.sh
3. Ensure SDL2 has built successfully.
4. Copy (or Link <- Symlinks seems not to work?!) the JoystickPlugin into /Engine/Plugins (result -> UnrealEngine/Engine/Plugins/JoystickPlugin)
5. ./GenerateProjectFiles.sh to create the makefile which compiles the JoystickPlugin with the whole engine
6. Compile
7. Start editor and enable the JoystickPlugin
8. If you have a compiled version of the plugin, you can copy/move the JoysticlPlugin directory to the project's plugins directory you want to use it.

### Note:
A first howto video to show how it should work.[YouTube](https://youtu.be/9SG73cxi5_A)
