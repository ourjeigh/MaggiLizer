#include "MaggiLizerFXDSP.h"
#include <math.h>

MaggiLizerFXDSP::MaggiLizerFXDSP() :
    m_pCachedBuffer(nullptr),
    m_pPlaybackBuffer(nullptr),
    m_uSampleRate(0),
    m_uBufferSampleSize(0),
    m_uCurrentCachedBufferSample(0),
    m_uPlaybackSampleHead(0)
{
}

MaggiLizerFXDSP::~MaggiLizerFXDSP()
{
}

void MaggiLizerFXDSP::Init(uint in_uSampleRate, float in_fSplice, uint in_numChannels)
{
    m_uSampleRate = in_uSampleRate;
    m_uCurrentCachedBufferSample = 0;
    m_uPlaybackSampleHead = 0;

    CaclulateBufferSampleSize(in_fSplice);

    m_pCachedBuffer = new float* [in_numChannels];
    m_pPlaybackBuffer = new float* [in_numChannels];

    for (uint i = 0; i < in_numChannels; i++)
    {
        uint cacheSize = (double)2 * m_uSampleRate;
        uint playbackSize = (double)4 * m_uSampleRate;
        m_pCachedBuffer[i] = new float[cacheSize]; // allow for a max of 2s of buffer
        m_pPlaybackBuffer[i] = new float[playbackSize]; // allow for a max of 2s of buffer played back half speed
        ClearBuffer(m_pCachedBuffer[i], cacheSize);
        ClearBuffer(m_pPlaybackBuffer[i], playbackSize);
    }
}

void MaggiLizerFXDSP::Term()
{
}

void MaggiLizerFXDSP::Reset()
{
}

void MaggiLizerFXDSP::Execute(
    float** io_pBuffer,
    const uint in_uNumChannels,
    const uint in_uValidFrames,
    const bool in_bReverse,
    const float in_fPitch,
    const float in_fSplice,
    const float in_fDelay,
    const float in_fRecycle,
    const float in_fMix)
{
    //Update Buffer Size
    CaclulateBufferSampleSize(in_fSplice);

    // Pitch change is just playback speed
    float speed = CalculateSpeed(in_fPitch);

    uint uFramesProcessed = 0;
    bool bufferFilled = false;

    // per frame loop
    while (uFramesProcessed < in_uValidFrames)
    {
        bufferFilled = false;
        // per channel loop
        for (uint channel = 0; channel < in_uNumChannels; ++channel)
        {
            ProcessSingleFrame(
                io_pBuffer, 
                m_pCachedBuffer, 
                m_pPlaybackBuffer, 
                channel, 
                uFramesProcessed, 
                bufferFilled, 
                speed, 
                in_bReverse, 
                in_fMix);
        }
        uFramesProcessed++;
        m_uCurrentCachedBufferSample++;
        m_uPlaybackSampleHead++;
    }
}

void MaggiLizerFXDSP::ProcessSingleFrame(
    float** io_pBuffer,
    float** in_pCachedBuffer,
    float** in_pPlaybackBuffer,
    const uint channel,
    const uint uFramesProcessed,
    bool& bufferFilled, 
    const float speed,
    const bool in_bReverse,
    const float in_fMix)
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
        ClearBuffer(m_pPlaybackBuffer[channel], 4 * m_uSampleRate);
        ApplyReverse(m_pCachedBuffer[channel], m_uBufferSampleSize, in_bReverse);
        ApplySpeed(m_pCachedBuffer[channel], m_pPlaybackBuffer[channel], m_uBufferSampleSize, speed);
        ClearBuffer(m_pCachedBuffer[channel], 2 * m_uSampleRate);
    }

    SetBufferValue(m_pCachedBuffer, channel, m_uCurrentCachedBufferSample, input);

    const float output = GetBufferValue(m_pPlaybackBuffer, channel, m_uPlaybackSampleHead);
    const float mixed = MixInputWithOutput(input, output, in_fMix);

    SetBufferValue(io_pBuffer, channel, uFramesProcessed, mixed);
}

void MaggiLizerFXDSP::SetBufferValue(float** io_pBuffer, const uint& in_uChannel, const uint in_uBufferSamplePosition,  const float in_fInput)
{
    io_pBuffer[in_uChannel][in_uBufferSamplePosition] = in_fInput;
}

float MaggiLizerFXDSP::GetBufferValue(float** in_pBuffer, const uint& in_channel, const uint& in_uBufferPosition)
{
    return in_pBuffer[in_channel][in_uBufferPosition];
}

/// <summary>
/// Description: Mix generated output value with input buffer value based on mix value.
/// </summary>
float MaggiLizerFXDSP::MixInputWithOutput(const float in_fInput, const float in_fOutput, const float in_fMix)
{
    return (in_fInput) * (1 - in_fMix) + in_fOutput * in_fMix;
}

void MaggiLizerFXDSP::SwapBufferValues(float* a, float* b)
{
    float temp = *a;
    *a = *b;
    *b = temp;
}

void MaggiLizerFXDSP::ReverseBuffer(float* io_pBuffer, uint in_uBufferSize)
{
    float* pointerLeft = io_pBuffer;
    float* pointerRight = io_pBuffer + in_uBufferSize - 1;

    while (pointerLeft < pointerRight) {
        SwapBufferValues(pointerLeft, pointerRight);
        pointerLeft++;
        pointerRight--;
    }
}

void MaggiLizerFXDSP::ApplyReverse(float* io_pBuffer, uint in_uBufferSize, bool in_bReverse)
{
    if (in_bReverse)
    {
        ReverseBuffer(io_pBuffer, in_uBufferSize);
    }
}


void MaggiLizerFXDSP::ApplySpeed(float* in_pBuffer, float* out_pBuffer, uint in_uBufferSize, float speed)
{
    // scale the output buffer size to adjust for more/less samples due to speed
    uint outBufferSize = in_uBufferSize;
    if (speed != 1)
    {
        in_uBufferSize = floor((float)outBufferSize / speed);
    }

    float position = 0.0;
    for (uint i = 0; i < outBufferSize; i++)
    {
        if (position >= in_uBufferSize - 1)
            return;

        uint intPos = floor(position);
        float floatPos = position - intPos;

        float prev = in_pBuffer[intPos];
        float next = in_pBuffer[intPos + 1];

        out_pBuffer[i] = (1 - floatPos) * prev + (floatPos * next);

        position += speed;
    }
}

void MaggiLizerFXDSP::ClearBuffer(float* buffer, uint bufferSize)
{
    for (uint i = 0; i < bufferSize; i++)
    {
        buffer[i] = 0;
    }
}

void MaggiLizerFXDSP::CaclulateBufferSampleSize(float splice)
{
    m_uBufferSampleSize = splice / (float)1000 * m_uSampleRate; // actual buffer based on splice size
}

float MaggiLizerFXDSP::CalculateSpeed(float in_fPitch) const
{
    return pow(2, in_fPitch / 1200);
}