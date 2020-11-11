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

void MaggiLizerFXDSP::Init(int in_uSampleRate, float in_fSplice, int in_numChannels)
{
    m_uSampleRate = in_uSampleRate;
    m_uCurrentCachedBufferSample = 0;
    m_uPlaybackSampleHead = 0;

    CaclulateBufferSampleSize(in_fSplice);

    m_pCachedBuffer = new float* [in_numChannels];
    m_pPlaybackBuffer = new float* [in_numChannels];

    for (int i = 0; i < in_numChannels; i++)
    {
        int cacheSize = (double)2 * m_uSampleRate;
        int playbackSize = (double)4 * m_uSampleRate;
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
    float** io_paBuffer,
    const int in_uNumChannels,
    const int in_uValidFrames,
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

    unsigned int uFramesProcessed = 0;
    bool bufferFilled = false;

    // per frame loop
    while (uFramesProcessed < in_uValidFrames)
    {
        bufferFilled = false;
        // per channel loop
        for (int channel = 0; channel < in_uNumChannels; ++channel)
        {
            ProcessSingleFrame(
                io_paBuffer, 
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

void MaggiLizerFXDSP::ProcessSingleFrame(float** io_paBuffer,
    float** in_pCachedBuffer,
    float** in_pPlaybackBuffer,
    const int channel,
    const unsigned int uFramesProcessed,
    bool& bufferFilled, 
    const float speed,
    const bool in_bReverse,
    const float in_fMix)
{
    float input = GetBufferValue(io_paBuffer, channel, uFramesProcessed);

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
        ApplySpeedAndReverse(m_pCachedBuffer[channel], m_pPlaybackBuffer[channel], m_uBufferSampleSize, speed, in_bReverse);
        ClearBuffer(m_pCachedBuffer[channel], 2 * m_uSampleRate);
    }

    m_pCachedBuffer[channel][m_uCurrentCachedBufferSample] = input;

    const float output = GetBufferValue(m_pPlaybackBuffer, channel, m_uPlaybackSampleHead);

    const float mixed = MixInputWithOutput(input, output, in_fMix);

    io_paBuffer[channel][uFramesProcessed] = mixed;
}

float MaggiLizerFXDSP::GetBufferValue(float**in_pBuffer, const int& in_channel, const unsigned int& in_uBufferPosition)
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

void SwapArrayValues(float* a, float* b)
{
    float temp = *a;
    *a = *b;
    *b = temp;
}

void ReverseArray(float* array, int array_size)
{
    float* pointerLeft = array;
    float* pointerRight = array + array_size - 1;

    while (pointerLeft < pointerRight) {
        SwapArrayValues(pointerLeft, pointerRight);
        pointerLeft++;
        pointerRight--;
    }
}

void MaggiLizerFXDSP::ApplySpeedAndReverse(float* inBuffer, float* outBuffer, int bufferSize, float speed, bool b_reverse)
{
    if (b_reverse)
    {
        ReverseArray(inBuffer, bufferSize);
    }
    
    // scale the output buffer size to adjust for more/less samples due to speed
    int outBufferSize = bufferSize;
    if (speed != 1)
    {
        bufferSize = floor((float)outBufferSize / speed);
    }

    float position = 0.0;
    for (int i = 0; i < outBufferSize; i++)
    {
        if (position >= bufferSize - 1)
            return;

        int intPos = floor(position);
        float floatPos = position - intPos;

        float prev = inBuffer[intPos];
        float next = inBuffer[intPos + 1];

        outBuffer[i] = (1 - floatPos) * prev + (floatPos * next);

        position += speed;
    }
}

void MaggiLizerFXDSP::ClearBuffer(float* buffer, int bufferSize)
{
    for (int i = 0; i < bufferSize; i++)
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