# MaggiLizer
Crystallizer knockoff Wwise Plugin
AKA
The Secret Sauce for making all your sound taste better.


## Download The Libraries
https://github.com/rjmattingly/MaggiLizer/releases
Both the xml and the dll go in {YourWwiseInstallDirectory}\Authoring\x64\Release\bin\Plugins

## Working with the Source

### Required Installs
- Wwise 2019.2.6
  - Requires 2 Deployment Platforms for Authoring plugin
     - Microsoft Windows Visual Studio 2015
     - Microsoft Windows Visual Studio 2019
- Python 3
- Visual Studio 2019
  - Additional Component to build Authoring plugin
    - Desktop development with C++ (Workload)
    - C++ ATL for v141 build tools (Individual Component)
    - C++ MFC for v141 build tools (Individual Component)

### Generating Solutions

- Locate wp.py
  - For Wwise 2019.2.6 ->  C:\Program Files (x86)\Audiokinetic\Wwise 2019.2.6.7381\Scripts\Build\Plugins\wp.py
- Open Command Prompt in MaggiLizer\MaggiLizer
- Run wp.py premake Authoring -Debug -x x64_vc160

### Building

- Locate wp.py
  - For Wwise 2019.2.6 ->  C:\Program Files (x86)\Audiokinetic\Wwise 2019.2.6.7381\Scripts\Build\Plugins\wp.py
- Open Command Prompt in MaggiLizer\MaggiLizer
- Run wp.py build Authoring -Debug -x x64_vc160
- Example with default directories:
  - "C:\Program Files (x86)\Audiokinetic\Wwise 2019.2.6.7381\Scripts\Build\Plugins\wp.py" build Authoring -c Debug -x x64_vc160
- Use "wp.py build --help" for more options and info

### Debugging

- Open MaggiLizer_Authoring_Windows_vc160.sln with Visual Studio 2019
- Select the MaggiLizer project in the Solution Explorer (not MaggiLizerFX)
- Open the Properties by right clicking or pressing Alt + Enter
- Change the configuration (top left) to Debug
- Change the Output Directory replacing '$Configuration' with 'Release'
  - This is because the Wwise Authoring app does not look in the x64\Debug directory for plugins, so they need to be put in Release.
- Open Wwise 2019 Authoring app
- In Visual Studio: Select Debug > Attach to Process (Ctrl + Alt + P)
- Select Wwise.exe and connect
- You can now add breakpoints to the code 
  - MaggiLizerFX::Execute is called every frame while the FX is processing audio so that's a good place.
  - MaggiLizerFX::Init is called every time you press play in the Authoring app, followed immediately by Reset.

### Wwise Plugin Documentation

- [Wwise Up On Air: Creating a Plug-In for Wwise](https://www.twitch.tv/videos/758625080)
- [How to Create Wwise Sound Engine Effect Plug-ins](https://www.audiokinetic.com/library/edge/?source=SDK&id=soundengine_plugins_effects.html)
