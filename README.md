# MaggiLizer
Crystallizer knockoff Wwise Plugin


## Download The Libraries
TODO

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

### Building

- Locate wp.py
 - For Wwise 2019.2.6 ->  C:\Program Files (x86)\Audiokinetic\Wwise 2019.2.6.7381\Scripts\Build\Plugins\wp.py
- Open Command Prompt in MaggiLizer\MaggiLizer
- Run wp.py build Authoring -Debug -x x64_vc160
- Example with default directories:
 - "C:\Program Files (x86)\Audiokinetic\Wwise 2019.2.6.7381\Scripts\Build\Plugins\wp.py" build Authoring -c Debug -x x64_vc160
- Use "wp.py build --help" for more options and info
