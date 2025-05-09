/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the
"Apache License"); you may not use this file except in compliance with the
Apache License. You may obtain a copy of the Apache License at
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Copyright (c) 2024 Audiokinetic Inc.
*******************************************************************************/

#include "maggilizerPlugin.h"
#include "../SoundEnginePlugin/maggilizerFXFactory.h"

maggilizerPlugin::maggilizerPlugin()
{
}

maggilizerPlugin::~maggilizerPlugin()
{
}

bool maggilizerPlugin::GetBankParameters(const GUID& in_guidPlatform, AK::Wwise::Plugin::DataWriter& in_dataWriter) const
{
    // Write bank data here
    // See maggilizer.xml property Names
    in_dataWriter.WriteReal32(m_propertySet.GetReal32(in_guidPlatform, "reverse"));
    in_dataWriter.WriteReal32(m_propertySet.GetReal32(in_guidPlatform, "pitch"));
    in_dataWriter.WriteReal32(m_propertySet.GetReal32(in_guidPlatform, "splice"));
    in_dataWriter.WriteReal32(m_propertySet.GetReal32(in_guidPlatform, "delay"));
    in_dataWriter.WriteReal32(m_propertySet.GetReal32(in_guidPlatform, "recycle"));
    in_dataWriter.WriteReal32(m_propertySet.GetReal32(in_guidPlatform, "mix"));

    return true;
}

DEFINE_AUDIOPLUGIN_CONTAINER(maggilizer);											// Create a PluginContainer structure that contains the info for our plugin
EXPORT_AUDIOPLUGIN_CONTAINER(maggilizer);											// This is a DLL, we want to have a standardized name
ADD_AUDIOPLUGIN_CLASS_TO_CONTAINER(                                             // Add our CLI class to the PluginContainer
    maggilizer,        // Name of the plug-in container for this shared library
    maggilizerPlugin,  // Authoring plug-in class to add to the plug-in container
    maggilizerFX       // Corresponding Sound Engine plug-in class
);
DEFINE_PLUGIN_REGISTER_HOOK

#ifdef _DEBUG
#define DEFINEDUMMYASSERTHOOK void AkAssertHookFunc( \
    const char* in_pszExpression,\
    const char* in_pszFileName,\
    int in_lineNumber)\
    {\
    __debugbreak();\
    __halt();\
    }\
    AkAssertHook g_pAssertHook = AkAssertHookFunc;
#endif // _DEBUG

DEFINEDUMMYASSERTHOOK;							// Placeholder assert hook for Wwise plug-ins using AKASSERT (cassert used by default)