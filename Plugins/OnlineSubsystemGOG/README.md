### Supported baseline:
- UnrealEngine 4.19.2
- Galaxy SDK 1.131.3

### Known issues and limitations:
- GalaxySDK may be initialized only once per process, so each player window must be spawned in a separate process
- A player has to be logged on to GOG backend services prior to using any features from OnlineSubsystemGOG
- Dedicated servers are not supported yet

# Installing plugin:

- Copy the plugin folder to your **"GameFolder/Plugins"**
- Unpack content of the [Galaxy SDK](https://devportal.gog.com/galaxy/components/sdk "Galaxy SDK") to **"GameFolder/Plugins/OnlineSubsystemGOG/Source/ThirdParty/GalaxySDK"**
- Enable the plugin:
	* either via UE4 Editor interface:
		* **Settings** -> **Plugins** -> **Online** -> **OnlineSubsystemGOG**
		* check **"Enabled"**
		* restart UnrealEditor

	* or modifying **&#42;.uproject** file manually as follows:
```
"Plugins":
[
	{
		"Name": "OnlineSubsystem",
		"Enabled": true
	},
	{
		"Name": "OnlineSubsystemGOG",
		"Enabled": true
	}
]
```
- Update default engine configuration file (**"GameFolder/Config/DefaultEngine.ini"**) with **ClientID** and **ClientSecret**:

```
[OnlineSubsystem]
DefaultPlatformService=GOG

[OnlineSubsystemGOG]
ClientID=<CLIENT_ID>
ClientSecret=<CLIENT_SECRET>

[/Script/Engine.Engine]
!NetDriverDefinitions=ClearArray
NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/OnlineSubsystemGOG.NetDriverGOG",DriverClassNameFallback="/Script/OnlineSubsystemUtils.IpNetDriver")

[/Script/Engine.GameEngine]
!NetDriverDefinitions=ClearArray
NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/OnlineSubsystemGOG.NetDriverGOG",DriverClassNameFallback="/Script/OnlineSubsystemUtils.IpNetDriver")

[/Script/OnlineSubsystemGOG.NetDriverGOG]
NetConnectionClassName="/Script/OnlineSubsystemGOG.NetConnectionGOG"
```
 Please contact your GOG.com tech representative for more info on how to obtain **ClientID** and **ClientSecret**

# Logging in:
- Using C++, IOnlineIdentity::Login() method is provided:

```
auto onlineIdentityInterface = Online::GetIdentityInterface(TEXT("GOG"));
auto onLoginCompleteDelegateHandle = onlineIdentityInterface->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateRaw(this, &OnLoginComplete));
FOnlineAccountCredentials accountCredentials;
onlineIdentityInterface->Login(0, accountCredentials);

void OnLoginComplete(int32, bool, const FUniqueNetId&, const FString&)
{
	Online::GetIdentityInterface(TEXT("GOG"))->ClearOnLoginCompleteDelegate_Handle(0, onLoginCompleteDelegateHandle);
	...
	// Proceed with online features initialization
}
```

- Using Blueprints, you can find **Login** method under the **Online** category

# Using the Achivements:
Prior to using the achivements:
* Achivements must be defined in [GOG Devportal](https://devportal.gog.com/panel/games "GOG Devportal")
* Achivements **API Key**s as defined in Devportal should be provided to engine configuration file:

```
[OnlineSubsystemGOG]

+Achievements=<ACHIEVEMENT_KEY_1>
+Achievements=<ACHIEVEMENT_KEY_2>
... e.t.c.
```
