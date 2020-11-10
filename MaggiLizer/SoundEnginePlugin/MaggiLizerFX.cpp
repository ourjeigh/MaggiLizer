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
    cachedBuffer(nullptr),
    playbackBuffer(nullptr),
    sampleRate(0),
    uBufferSampleSize(0),
    uCurrentCachedBufferSample(0),
    uPlaybackSampleHead(0)
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

    sampleRate = in_rFormat.uSampleRate;
    uCurrentCachedBufferSample = 0;
    CalculateBufferSampleSize(m_pParams);

    int numChannels = in_rFormat.GetNumChannels();
    cachedBuffer = new AkReal32* [numChannels];
    playbackBuffer = new AkReal32* [numChannels];

    for (int i = 0; i < numChannels; i++)
    {
        cachedBuffer[i] = new AkReal32[2 * sampleRate]; // allow for a max of 2s of buffer
        playbackBuffer[i] = new AkReal32[4 * sampleRate]; // allow for a max of 2s of buffer played back half speed
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
    int localBufferPosition = 0;
    // Pitch change is just playback speed
    AkReal32 pitch = m_pParams->RTPC.fPitch;
    float speed = pow(2, pitch / 1200);

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
            float input = pBuf[curCh][uFramesProcessed];

            /// if cachedBuffer is full move it to playbackBuffer and clear
            if (uCurrentCachedBufferSample + localBufferPosition >= uBufferSampleSize)
            {
                // use flag to let subsequent channels perform the full buffer tasks.
                cacheFilled = true;
            }
            if (cacheFilled)
            {
                ClearBuffer(playbackBuffer[curCh], 4 * sampleRate);
                ApplySpeedAndReverse(cachedBuffer[curCh], playbackBuffer[curCh], uBufferSampleSize, speed, m_pParams->RTPC.bReverse);
                ClearBuffer(cachedBuffer[curCh], 2 * sampleRate);
                uCurrentCachedBufferSample = 0;
                uPlaybackSampleHead = 0;
                localBufferPosition = 0;
            }
            /// 
            
            cachedBuffer[curCh][uCurrentCachedBufferSample + localBufferPosition] = input;

            float output = 0; //initialize to zero for early input where playbackBuffer isn't ready.

            // if the playbackBuffer samples are greater/less than +/-1 they're garbage values, output silence instead.
            if (playbackBuffer != nullptr && fabs((playbackBuffer[curCh][uPlaybackSampleHead + localBufferPosition])) <= 1)
            {
                output = playbackBuffer[curCh][uPlaybackSampleHead + localBufferPosition];
            }

            // mix generated output with input buffer based on mix value.
            AkReal32 mixed = (input) * (1 - m_pParams->RTPC.fMix) + output * m_pParams->RTPC.fMix;

            pBuf[curCh][uFramesProcessed] = mixed;
        }
        uFramesProcessed++;
        localBufferPosition++;
    }

    uPlaybackSampleHead += uFramesProcessed - 1;
    uCurrentCachedBufferSample += uFramesProcessed - 1;
}

void SwapArrayValues(AkReal32* a, AkReal32* b)
{
    AkReal32 temp = *a;
    *a = *b;
    *b = temp;
}

void ReverseArray(AkReal32* array, int array_size)
{
    AkReal32* pointerLeft = array;
    AkReal32* pointerRight = array + array_size - 1;

    while (pointerLeft < pointerRight) {
        SwapArrayValues(pointerLeft, pointerRight);
        pointerLeft++;
        pointerRight--;
    }
}

void MaggiLizerFX::ApplySpeedAndReverse(AkReal32* inBuffer, AkReal32* outBuffer, int bufferSize, float speed, bool b_reverse)
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

void MaggiLizerFX::ClearBuffer(AkReal32* buffer, int bufferSize)
{
    for (int i = 0; i < bufferSize; i++)
    {
        buffer[i] = 0;
    }
}

void MaggiLizerFX::CalculateBufferSampleSize(AK::IAkPluginParam* in_pParams)
{
    uBufferSampleSize = m_pParams->RTPC.fSplice / 1000 * sampleRate; // actual buffer based on splice size
}
