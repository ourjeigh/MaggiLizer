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
    m_paCachedBuffer(nullptr),
    m_paPlaybackBuffer(nullptr),
    m_uSampleRate(0),
    m_uBufferSampleSize(0),
    m_uCurrentCachedBufferSample(0),
    m_uPlaybackSampleHead(0)
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

    m_uSampleRate = in_rFormat.uSampleRate;
    m_uCurrentCachedBufferSample = 0;
    CalculateBufferSampleSize(m_pParams);

    int numChannels = in_rFormat.GetNumChannels();
    m_paCachedBuffer = new AkReal32* [numChannels];
    m_paPlaybackBuffer = new AkReal32* [numChannels];

    for (int i = 0; i < numChannels; i++)
    {
        m_paCachedBuffer[i] = new AkReal32[(double)2 * m_uSampleRate]; // allow for a max of 2s of buffer
        m_paPlaybackBuffer[i] = new AkReal32[(double)4 * m_uSampleRate]; // allow for a max of 2s of buffer played back half speed
    }
    

    return AK_Success;
}

AKRESULT MaggiLizerFX::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT MaggiLizerFX::Reset()
{
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
    //Update Buffer Size
    CalculateBufferSampleSize(m_pParams);

    const AkUInt32 uNumChannels = io_pBuffer->NumChannels();
    AkUInt16 uFramesProcessed = 0;
    AkUInt16 localBufferPosition = 0;

    // Pitch change is just playback speed
    AkReal32 pitch = m_pParams->RTPC.fPitch;
    AkReal32 speed = pow(2, pitch / 1200);

    AkReal32** AK_RESTRICT pBuf = new AkReal32 * AK_RESTRICT[uNumChannels];
    for (int i = 0; i < uNumChannels; i++)
    {
        pBuf[i] = (AkReal32 * AK_RESTRICT)io_pBuffer->GetChannel(i);
    }

    while (uFramesProcessed < io_pBuffer->uValidFrames)
    {
        bool cacheFilled = false;
        for (AkUInt32 curCh = 0; curCh < uNumChannels; ++curCh)
        {
            AkReal32 input = pBuf[curCh][uFramesProcessed];

            /// if cachedBuffer is full move it to playbackBuffer and clear
            if (m_uCurrentCachedBufferSample + localBufferPosition >= m_uBufferSampleSize)
            {
                // use flag to let subsequent channels perform the full buffer tasks.
                cacheFilled = true;
            }
            if (cacheFilled)
            {
                ClearBuffer(m_paPlaybackBuffer[curCh], 4 * m_uSampleRate);
                ApplySpeedAndReverse(m_paCachedBuffer[curCh], m_paPlaybackBuffer[curCh], m_uBufferSampleSize, speed, m_pParams->RTPC.bReverse);
                ClearBuffer(m_paCachedBuffer[curCh], 2 * m_uSampleRate);
                m_uCurrentCachedBufferSample = 0;
                m_uPlaybackSampleHead = 0;
                localBufferPosition = 0;
            }
            /// 
            
            m_paCachedBuffer[curCh][m_uCurrentCachedBufferSample + localBufferPosition] = input;

            AkReal32 output = 0; //initialize to zero for early input where playbackBuffer isn't ready.

            // if the playbackBuffer samples are greater/less than +/-1 they're garbage values, output silence instead.
            if (m_paPlaybackBuffer != nullptr && fabs((m_paPlaybackBuffer[curCh][m_uPlaybackSampleHead + localBufferPosition])) <= 1)
            {
                output = m_paPlaybackBuffer[curCh][m_uPlaybackSampleHead + localBufferPosition];
            }

            // mix generated output with input buffer based on mix value.
            AkReal32 mixed = (input) * (1 - m_pParams->RTPC.fMix) + output * m_pParams->RTPC.fMix;

            pBuf[curCh][uFramesProcessed] = mixed;
        }
        uFramesProcessed++;
        localBufferPosition++;
    }

    m_uPlaybackSampleHead += uFramesProcessed - 1;
    m_uCurrentCachedBufferSample += uFramesProcessed - 1;
}

void MaggiLizerFX::SwapArrayValues(AkReal32* left, AkReal32* right)
{
    AkReal32 temp = *left;
    *left = *right;
    *right = temp;
}

void MaggiLizerFX::ReverseArray(AkReal32* array, AkUInt16 array_size)
{
    AkReal32* pointerLeft = array;
    AkReal32* pointerRight = array + array_size - 1;

    while (pointerLeft < pointerRight) {
        SwapArrayValues(pointerLeft, pointerRight);
        pointerLeft++;
        pointerRight--;
    }
}

void MaggiLizerFX::ApplySpeedAndReverse(AkReal32* inBuffer, AkReal32* outBuffer, AkUInt16 bufferSize, AkReal32 speed, bool b_reverse)
{
    float position = 0.0;
    if (b_reverse)
    {
        ReverseArray(inBuffer, bufferSize);
    }
    // TODO: If speed is changing the outBuffer size should change as well, and iteration should change
    // to fill exactly the needed buffer size.
    // -- issue: https://github.com/rjmattingly/MaggiLizer/projects/1#card-49019952
    for (int i = 0; i < bufferSize; i++)
    {
        if (position >= bufferSize - 1)
            return;

        int intPos = floor(position);
        float floatPos = position - intPos;

        AkReal32 prev = inBuffer[intPos];
        AkReal32 next = inBuffer[intPos + 1];

        outBuffer[i] = (1 - floatPos) * prev + (floatPos * next);

        position += speed;
    }
}

void MaggiLizerFX::ClearBuffer(AkReal32* buffer, AkUInt16 bufferSize)
{
    for (int i = 0; i < bufferSize; i++)
    {
        buffer[i] = 0;
    }
}

void MaggiLizerFX::CalculateBufferSampleSize(AK::IAkPluginParam* in_pParams)
{
    m_uBufferSampleSize = m_pParams->RTPC.fSplice / 1000 * m_uSampleRate; // actual buffer based on splice size
}
