#include "MaggiLizerFXDSP.h"
#include <math.h>

MaggiLizerFXDSP::MaggiLizerFXDSP() :
    m_pCachedBuffer(nullptr),
    m_pPlaybackBuffer(nullptr),
    m_uSampleRate(0),
    m_uMaxBufferSize(0),
    m_uBufferSampleSize(0),
    m_uCurrentCachedBufferSample(0),
    m_uPlaybackSampleHead(0)
{
}

MaggiLizerFXDSP::~MaggiLizerFXDSP()
{
}

void MaggiLizerFXDSP::Init(uint in_uSampleRate, uint in_numChannels)
{
    m_uSampleRate = in_uSampleRate;
    m_uMaxBufferSize = 4 * m_uSampleRate; //Buffer Size needs to be up to 2s (max splice value of 2000ms) played back at half speed
    m_uCurrentCachedBufferSample = 0;
    m_uPlaybackSampleHead = 0;

    m_pCachedBuffer = new float* [in_numChannels];
    m_pPlaybackBuffer = new float* [in_numChannels];

    for (uint i = 0; i < in_numChannels; i++)
    {
        m_pCachedBuffer[i] = new float[m_uMaxBufferSize]; // cached size needs to equal playback size to handle recycle copy
        m_pPlaybackBuffer[i] = new float[m_uMaxBufferSize];
        ClearBufferSingle(m_pCachedBuffer[i], m_uMaxBufferSize);
        ClearBufferSingle(m_pPlaybackBuffer[i], m_uMaxBufferSize);
    }
}

void MaggiLizerFXDSP::Term()
{
}

void MaggiLizerFXDSP::Reset()
{
}

void MaggiLizerFXDSP::Execute(
    buffer io_pBuffer,
    const uint in_uNumChannels,
    const uint in_uValidFrames,
    const bool& in_bReverse,
    const float& in_fPitch,
    const float& in_fSplice,
    const float& in_fDelay,
    const float& in_fRecycle,
    const float& in_fMix)
{
    //Update Buffer Size
    m_uBufferSampleSize = ConvertMillisecondsToSamples(m_uSampleRate, in_fSplice);

    float fSpeed = CalculateSpeed(in_fPitch);

    uint uFramesProcessed = 0;
    bool bBufferFilled = false;

    // per frame loop
    while (uFramesProcessed < in_uValidFrames)
    {
        bBufferFilled = false;
        // per channel loop
        for (uint uChannel = 0; uChannel < in_uNumChannels; ++uChannel)
        {
            ProcessSingleFrame(
                io_pBuffer, 
                m_pCachedBuffer, 
                m_pPlaybackBuffer, 
                uChannel, 
                uFramesProcessed, 
                fSpeed, 
                in_bReverse, 
                in_fRecycle,
                in_fMix,
                bBufferFilled);
        }
        uFramesProcessed++;
        m_uCurrentCachedBufferSample++;
        m_uPlaybackSampleHead++;
    }
}

void MaggiLizerFXDSP::ProcessSingleFrame(
    buffer io_pBuffer,
    buffer in_pCachedBuffer,
    buffer in_pPlaybackBuffer,
    const uint& channel,
    const uint& uFramesProcessed, 
    const float& speed,
    const bool& in_bReverse,
    const float& in_fRecycle,
    const float& in_fMix,
    bool& bufferFilled)
{
    float input = GetBufferValue(io_pBuffer, channel, uFramesProcessed);

    // handle multiple channels by setting a flag that the buffer is filled
    // in channel 0, signaling to copy buffers and clear for all subsequent channels.
    if (m_uCurrentCachedBufferSample >= m_uBufferSampleSize)
    {
        bufferFilled = true;
        m_uCurrentCachedBufferSample = 0;
        m_uPlaybackSampleHead = 0;
    }
    if (bufferFilled)
    {
        const float lastCachedValue = GetBufferValue(m_pCachedBuffer,channel, m_uBufferSampleSize);
        ClearBufferSingle(m_pPlaybackBuffer[channel], m_uMaxBufferSize);
        ApplyReverseBufferSingle(m_pCachedBuffer[channel], m_uBufferSampleSize, in_bReverse);
        ApplySpeedBufferSingle(m_pCachedBuffer[channel], m_pPlaybackBuffer[channel], m_uBufferSampleSize, speed);
        ApplySmoothBufferSingle(m_pPlaybackBuffer[channel], lastCachedValue, m_cSmoothWindowSize);
        // Copy playbackBuffer back into cachedBuffer after all the modification is done so that it can be recycled with new input
        CopyBufferSingleValues(m_pPlaybackBuffer[channel], m_pCachedBuffer[channel], m_uMaxBufferSize);
    }
    
    ApplyRecycleBufferValue(m_pCachedBuffer, channel, m_uCurrentCachedBufferSample, input, in_fRecycle);

    const float output = GetBufferValue(m_pPlaybackBuffer, channel, m_uPlaybackSampleHead);
    const float mixed = CalculateWetDryMix(input, output, in_fMix);

    //TODO: Check for instantaneous pops based on the new mixed value being "drastically" 
    // different from this channel's last frame value.
    // - issue: https://github.com/rjmattingly/MaggiLizer/projects/1#card-49461406

    SetBufferValue(io_pBuffer, channel, uFramesProcessed, mixed);
}

// Recycle using existing cachedBuffer value combined with new input value.
// 2 options:
// - Option 1: Mix 2 signals as wet/dry based on Recycle percent - max output volume of 1 and 100% Recycle values seem to decay to 0.
// - Option 2: Use full input signal, add Recycled signal on top with a volume multiplied by Recycle Percent. Max volume > 1
// Since my experience with Recycle/Feedback parameters is that 100% values cause a much louder signal, I think Option 2 is
// the way to go here.
void MaggiLizerFXDSP::ApplyRecycleBufferValue(buffer io_pBuffer, const uint& channel, const uint& samplePosition, const float& input, const float& in_fRecycle)
{
    SetBufferValue(io_pBuffer, channel, samplePosition,
        //Option 1:
        //CalculateWetDryMix(input, GetBufferValue(m_pCachedBuffer, channel, m_uCurrentCachedBufferSample), in_fRecycle)
        //Option 2:
        input + GetBufferValue(io_pBuffer, channel, samplePosition) * in_fRecycle
    );
}

void MaggiLizerFXDSP::ApplySmoothBufferSingle(buffer_single io_pBuffer, const float& in_fStartValue, const uint& in_uSmoothWindowSize)
{
    for (uint i = 0; i < in_uSmoothWindowSize; i++)
    {
        const float weight = 1.0 - ((float)i/(float)in_uSmoothWindowSize);
        const float smoothedValue = CalculateWetDryMix(io_pBuffer[i], in_fStartValue, weight);
        io_pBuffer[i] = smoothedValue;
    }
}

void MaggiLizerFXDSP::SetBufferValue(buffer io_pBuffer, const uint& in_uChannel, const uint& in_uBufferSamplePosition, const float& in_fInput)
{
    io_pBuffer[in_uChannel][in_uBufferSamplePosition] = in_fInput;
}

float MaggiLizerFXDSP::GetBufferValue(buffer in_pBuffer, const uint& in_channel, const uint& in_uBufferPosition) const
{
    return in_pBuffer[in_channel][in_uBufferPosition];
}

void MaggiLizerFXDSP::SwapBufferValues(float* a, float* b)
{
    float temp = *a;
    *a = *b;
    *b = temp;
}

void MaggiLizerFXDSP::ApplyReverseBufferSingle(buffer_single io_pBuffer, const uint& in_uBufferSize, const bool& in_bReverse)
{
    if (!in_bReverse) return;

    float* pointerLeft = io_pBuffer;
    float* pointerRight = io_pBuffer + in_uBufferSize - 1;

    while (pointerLeft < pointerRight) 
    {
        SwapBufferValues(pointerLeft, pointerRight);
        pointerLeft++;
        pointerRight--;
    }
}

/// <summary>
/// - Resize the output buffer length if speed isn't 1.
/// - Iterate input buffer
/// - Interpolate values due to speed change
/// - Copy interpolated value to output buffer.
/// </summary>
void MaggiLizerFXDSP::ApplySpeedBufferSingle(buffer_single in_pBuffer, buffer_single out_pBuffer, const uint& in_uBufferSize, const float& in_fSpeed)
{
    uint uOutBufferSize = CalculateBufferSizeChangeFromSpeed(in_uBufferSize, in_fSpeed);

    float position = 0.0;
    for (uint i = 0; i < uOutBufferSize; i++)
    {
        // can't go past in_uBufferSize since that's the length of the input buffer.
        if (position >= in_uBufferSize - 1)
        {
            // If we were to set position = 0 here it would start repeating the input signal for speeds > 1.
            // This doesn't sound very good, so we just leave the rest of the buffer empty.
            return;
        }

        uint intPos = floor(position);
        float floatPos = position - intPos;

        float prev = in_pBuffer[intPos];
        float next = in_pBuffer[intPos + 1];

        out_pBuffer[i] = (1 - floatPos) * prev + (floatPos * next);

        position += in_fSpeed;
    }
}

/// <summary>
/// Instead of creating new arrays, just zero them out to prevent garbage values from being outputed. 
/// 0's will just be silence.
/// </summary>
void MaggiLizerFXDSP::ClearBufferSingle(buffer_single buffer, const uint& bufferSize)
{
    for (uint i = 0; i < bufferSize; i++)
    {
        buffer[i] = 0;
    }
}

void MaggiLizerFXDSP::CopyBufferSingleValues(buffer_single in_pBuffer, buffer_single out_pBuffer, const uint& in_uBufferSize)
{
    for (uint i = 0; i < in_uBufferSize; i++)
    {
        out_pBuffer[i] = in_pBuffer[i];
    }
}


/// <summary>
/// Scale the output buffer size to adjust for more/less samples due to speed if speed isn't 1.
/// </summary>
unsigned int MaggiLizerFXDSP::CalculateBufferSizeChangeFromSpeed(const uint& in_uBufferSize, const float& in_fSpeed) const
{
    return floor((float)in_uBufferSize / in_fSpeed);
}

/// <summary>
/// Actual buffer size for iteration is based on splice value
/// </summary>
unsigned int MaggiLizerFXDSP::ConvertMillisecondsToSamples(const uint& in_uSampleRate, const float& in_fMilliseconds) const
{
    // in_fSplice is in milliseconds, divide by 1000 to get seconds.
    return (uint)(in_fMilliseconds / (float)1000 * in_uSampleRate);
}

/// <summary>
/// Pitch change is just playback speed
/// </summary>
float MaggiLizerFXDSP::CalculateSpeed(const float& in_fPitch) const
{
    return pow(2, in_fPitch / 1200);
}

/// <summary>
/// Description: Mix generated output value with input buffer value based on mix value.
/// </summary>
float MaggiLizerFXDSP::CalculateWetDryMix(const float& in_fDry, const float& in_fWet, const float in_fMix) const
{
    return (in_fDry) * (1 - in_fMix) + in_fWet * in_fMix;
}