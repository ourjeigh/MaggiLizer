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

#ifndef maggilizerFXParams_H
#define maggilizerFXParams_H

#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Plugin/PluginServices/AkFXParameterChangeHandler.h>

// Add parameters IDs here, those IDs should map to the AudioEnginePropertyID
// attributes in the xml property definition.
enum e_param_ids
{
    param_id_reverse,
    param_id_pitch,
    param_id_splice,
    param_id_delay,
    param_id_recycle,
    param_id_mix,
    param_id_smoothing,

    k_param_id_count
};

struct maggilizerRTPCParams
{
    bool bReverse;
    AkReal32 fPitch;
    AkReal32 fSplice;
    AkReal32 fDelay;
    AkReal32 fRecycle;
    AkReal32 fMix;
	AkReal32 fSmoothing;

    void clear()
    {
        bReverse = false;
        fPitch = 0.0f;
        fSplice = 0.0f;
        fDelay = 0.0f;
        fRecycle = 0.0f;
        fMix = 0.0f;
		fSmoothing = 0.0f;
    }
};

struct maggilizerFXParams
    : public AK::IAkPluginParam
{
    maggilizerFXParams();
    maggilizerFXParams(const maggilizerFXParams& in_rParams);

    ~maggilizerFXParams();

    /// Create a duplicate of the parameter node instance in its current state.
    IAkPluginParam* Clone(AK::IAkPluginMemAlloc* in_pAllocator) override;

    /// Initialize the plug-in parameter node interface.
    /// Initializes the internal parameter structure to default values or with the provided parameter block if it is valid.
    AKRESULT Init(AK::IAkPluginMemAlloc* in_pAllocator, const void* in_pParamsBlock, AkUInt32 in_ulBlockSize) override;

    /// Called by the sound engine when a parameter node is terminated.
    AKRESULT Term(AK::IAkPluginMemAlloc* in_pAllocator) override;

    /// Set all plug-in parameters at once using a parameter block.
    AKRESULT SetParamsBlock(const void* in_pParamsBlock, AkUInt32 in_ulBlockSize) override;

    /// Update a single parameter at a time and perform the necessary actions on the parameter changes.
    AKRESULT SetParam(AkPluginParamID in_paramID, const void* in_pValue, AkUInt32 in_ulParamSize) override;

    AK::AkFXParameterChangeHandler<k_param_id_count> m_paramChangeHandler;

    maggilizerRTPCParams m_rtpcs;
};

#endif // maggilizerFXParams_H
