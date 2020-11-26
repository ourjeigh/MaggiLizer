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

  Copyright (c) 2020 Audiokinetic Inc.
*******************************************************************************/

#include "MaggiLizerFX.h"
#include "../MaggiLizerConfig.h"

#include <AK/AkWwiseSDKVersion.h>
#include <math.h>

AK::IAkPlugin* CreateMaggiLizerFX(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, MaggiLizerFX());
}

AK::IAkPluginParam* CreateMaggiLizerFXParams(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, MaggiLizerFXParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(MaggiLizerFX, AkPluginTypeEffect, MaggiLizerConfig::CompanyID, MaggiLizerConfig::PluginID)

MaggiLizerFX::MaggiLizerFX() :
    m_pParams(nullptr),
    m_pAllocator(nullptr),
    m_pContext(nullptr),
    m_pDSP(nullptr)
{
}

MaggiLizerFX::~MaggiLizerFX()
{
}

AKRESULT MaggiLizerFX::Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkEffectPluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat)
{
    m_pParams = (MaggiLizerFXParams*)in_pParams;
    m_pAllocator = in_pAllocator;
    m_pContext = in_pContext;

    m_pDSP = new MaggiLizerFXDSP();
    m_pDSP->Init(in_rFormat.uSampleRate, in_rFormat.GetNumChannels(), m_pParams->RTPC.fSplice);

    return AK_Success;
}

AKRESULT MaggiLizerFX::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    m_pDSP->Term();
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT MaggiLizerFX::Reset()
{
    m_pDSP->Reset();
    return AK_Success;
}

AKRESULT MaggiLizerFX::GetPluginInfo(AkPluginInfo& out_rPluginInfo)
{
    out_rPluginInfo.eType = AkPluginTypeEffect;
    out_rPluginInfo.bIsInPlace = true;
    out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
    return AK_Success;
}

void MaggiLizerFX::Execute(AkAudioBuffer* io_pBuffer)
{
    const int& numChannels = io_pBuffer->NumChannels();
    float** io_paBuffer = new float* [numChannels];

    for (int i = 0; i < numChannels; i++)
    {
        io_paBuffer[i] = io_pBuffer->GetChannel(i);
    }

    m_pDSP->Execute(io_paBuffer, 
                   numChannels,
                   io_pBuffer->uValidFrames,
                   m_pParams->RTPC.bReverse,
                   m_pParams->RTPC.fPitch,
                   m_pParams->RTPC.fSplice,
                   m_pParams->RTPC.fDelay,
                   m_pParams->RTPC.fRecycle,
                   m_pParams->RTPC.fMix);
}


