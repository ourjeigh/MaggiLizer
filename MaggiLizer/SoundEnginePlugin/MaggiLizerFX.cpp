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

#include "maggilizerFX.h"
#include "../maggilizerConfig.h"

#include <AK/AkWwiseSDKVersion.h>
#include "buffer_utilies.h"

typedef AkReal32* AK_RESTRICT AKReal32_Restrict;

const AkReal32 k_max_delay_seconds = 2.0f;
const AkUInt16 k_smoothing_window_samples= 30;

AK::IAkPlugin* CreatemaggilizerFX(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, maggilizerFX());
}

AK::IAkPluginParam* CreatemaggilizerFXParams(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, maggilizerFXParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(maggilizerFX, AkPluginTypeEffect, maggilizerConfig::CompanyID, maggilizerConfig::PluginID)

maggilizerFX::maggilizerFX()
    : m_pParams(nullptr)
    //, m_pContext(nullptr)
{
}

maggilizerFX::~maggilizerFX()
{
}

AKRESULT maggilizerFX::Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkEffectPluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat)
{
    m_pParams = static_cast<maggilizerFXParams*>(in_pParams);
    m_uSampleRate = in_rFormat.uSampleRate;
    const AkUInt32 uMaxBufferSize = 6 * m_uSampleRate;

    const int channel_count = in_rFormat.GetNumChannels();

    m_pSpliceBuffer = new LinearMonoBuffer * [channel_count];
    m_pPlaybackBuffer = new CircularMonoBuffer * [channel_count];


    for (AkUInt32 i = 0; i < channel_count; i++)
    {
        const AkUInt32 uSpliceSampleSize = MonoBufferUtilities::ConvertMillisecondsToSamples(m_uSampleRate, 2.0f);
        m_pSpliceBuffer[i] = new LinearMonoBuffer(uSpliceSampleSize);
        m_pPlaybackBuffer[i] = new CircularMonoBuffer(uMaxBufferSize);
    }


    AkUInt32 sample_count = k_max_delay_seconds * m_uSampleRate;
    AKRESULT result = m_delay.Init(in_pAllocator, sample_count, channel_count);

    return result;
}

AKRESULT maggilizerFX::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    // we are NOT responsible for freeing the cached params (and Wwise will crash if we do)
    
    m_delay.Term(in_pAllocator);

    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT maggilizerFX::Reset()
{
    m_delay.Reset();

    return AK_Success;
}

AKRESULT maggilizerFX::GetPluginInfo(AkPluginInfo& out_rPluginInfo)
{
    out_rPluginInfo.eType = AkPluginTypeEffect;
    out_rPluginInfo.bIsInPlace = true;
	out_rPluginInfo.bCanProcessObjects = false;
    out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
    return AK_Success;
}

void maggilizerFX::Execute(AkAudioBuffer* io_pBuffer)
{
    const AkUInt32 channel_count = io_pBuffer->NumChannels();

    AkUInt16 frames_processed;

    for (AkUInt32 channel_index = 0; channel_index < channel_count; channel_index++)
    {
        AKReal32_Restrict buffer = static_cast<AKReal32_Restrict>(io_pBuffer->GetChannel(channel_index));

        for (AkUInt32 frame_index = 0; frame_index < io_pBuffer->uValidFrames; frame_index++)
        {
            ProcessSingleFrame(
                buffer,
                m_pSpliceBuffer[channel_index],
                m_pPlaybackBuffer[channel_index],
                frame_index,
                m_pParams->m_rtpcs.bReverse,
                m_pParams->m_rtpcs.fPitch,
                m_pParams->m_rtpcs.fSplice,
                m_pParams->m_rtpcs.fDelay,
                m_pParams->m_rtpcs.fRecycle,
                m_pParams->m_rtpcs.fMix
            );

        }
    }
}

void maggilizerFX::ProcessSingleFrame(
    float* io_pBuffer,
    MonoBuffer* io_pSpliceBuffer,
    MonoBuffer* io_pPlaybackBuffer,
    const AkUInt32& in_uFrame,
    const bool& in_bReverse,
    const float& in_fPitch,
    const float& in_fSplice,
    const float& in_fDelay,
    const float& in_fRecycle,
    const float& in_fMix)
{
    float input = io_pBuffer[in_uFrame];

    // Delay
    if (in_fDelay > 0)
    {
        const AkUInt32 delaySampleSize = MonoBufferUtilities::ConvertMillisecondsToSamples(m_uSampleRate, in_fDelay);
        io_pPlaybackBuffer->SetReadDelay(delaySampleSize);
    }

    //Splice
    if (io_pSpliceBuffer->IsFilled())
    {
        // Reverse 
        if (in_bReverse)
        {
            MonoBufferUtilities::ApplyReverseBufferSingle(io_pSpliceBuffer);
        }

        // Pitch
        const AkUInt32 playbackWritePosition = io_pPlaybackBuffer->WritePosition();
        const float fSpeed = MonoBufferUtilities::CalculateSpeed(in_fPitch);
        const AkUInt32 filledPlaybackSampleSize = MonoBufferUtilities::ApplySpeedBufferSingle(io_pSpliceBuffer, io_pPlaybackBuffer, fSpeed);

        //MonoBufferUtilities::ApplySmoothingAtIndex(io_pPlaybackBuffer, playbackWritePosition, m_cSmoothWindowSize);
        const AkUInt32 uSpliceSampleSize = MonoBufferUtilities::ConvertMillisecondsToSamples(m_uSampleRate, in_fSplice);
        io_pSpliceBuffer->SetBufferSize(uSpliceSampleSize);

        //Recyle
        MonoBufferUtilities::CopyLastWrittenBufferBlock(io_pPlaybackBuffer, io_pSpliceBuffer, filledPlaybackSampleSize);
    }

    float recycled = 0;
    io_pSpliceBuffer->ReadNextBufferValue(recycled);
    recycled = input + recycled * in_fRecycle;

    io_pSpliceBuffer->WriteNextBufferValue(recycled);

    float output = 0;
    if (io_pPlaybackBuffer->HasData())
    {
        io_pPlaybackBuffer->ReadNextBufferValue(output);
    }

    float mix = MonoBufferUtilities::CalculateWetDryMix(input, output, in_fMix);

    io_pBuffer[in_uFrame] = mix;
}

AKRESULT maggilizerFX::TimeSkip(AkUInt32 in_uFrames)
{
    return AK_DataReady;
}
