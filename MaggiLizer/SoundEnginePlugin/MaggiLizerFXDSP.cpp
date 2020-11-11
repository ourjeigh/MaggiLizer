#include "MaggiLizerFXDSP.h"
#include <math.h>

MaggiLizerFXDSP::MaggiLizerFXDSP() :
    cachedBuffer(nullptr),
    playbackBuffer(nullptr),
    sampleRate(0),
    uBufferSampleSize(0),
    uCurrentCachedBufferSample(0),
    uPlaybackSampleHead(0)
{
}

MaggiLizerFXDSP::~MaggiLizerFXDSP()
{
}

void MaggiLizerFXDSP::Init(int in_uSampleRate, float in_fSplice, int in_numChannels)
{
    sampleRate = in_uSampleRate;
    uCurrentCachedBufferSample = 0;
    CaclulateBufferSampleSize(in_fSplice);

    cachedBuffer = new float* [in_numChannels];
    playbackBuffer = new float* [in_numChannels];

    for (int i = 0; i < in_numChannels; i++)
    {
        cachedBuffer[i] = new float[(double)2 * sampleRate]; // allow for a max of 2s of buffer
        playbackBuffer[i] = new float[(double)4 * sampleRate]; // allow for a max of 2s of buffer played back half speed
    }
}

void MaggiLizerFXDSP::Term()
{
}

void MaggiLizerFXDSP::Reset()
{
}

void MaggiLizerFXDSP::Execute(float** io_paBuffer,
                              int in_uNumChannels,
                              int in_uValidFrames,
                              bool in_bReverse,
                              float in_fPitch,
                              float in_fSplice,
                              float in_fDelay,
                              float in_fRecycle,
                              float in_fMix)
{
    //Update Buffer Size
    CaclulateBufferSampleSize(in_fSplice);


    // Pitch change is just playback speed
    float speed = pow(2, in_fPitch / 1200);

    unsigned int uFramesProcessed = 0;
    int localBufferPosition = 0;
    bool bufferFilled = false;

    while (uFramesProcessed < in_uValidFrames)
    {
        bufferFilled = false;
        for (int channel = 0; channel < in_uNumChannels; ++channel)
        {

            float input = io_paBuffer[channel][uFramesProcessed];

            // handle multiple channels by setting a flag that the buffer is filled
            // in channel 0, signaling to copy buffers and clear for all subsequent channels.
            if (uCurrentCachedBufferSample + localBufferPosition >= uBufferSampleSize)
            {
                bufferFilled = true;
                uCurrentCachedBufferSample = 0;
                uPlaybackSampleHead = 0;
                localBufferPosition = 0;
            }
            if (bufferFilled)
            {
                ClearBuffer(playbackBuffer[channel], 4 * sampleRate);
                ApplySpeedAndReverse(cachedBuffer[channel], playbackBuffer[channel], uBufferSampleSize, speed, in_bReverse);
                ClearBuffer(cachedBuffer[channel], 2 * sampleRate);
            }

            cachedBuffer[channel][uCurrentCachedBufferSample + localBufferPosition] = input;

            float output = 0;

            // if the playbackBuffer samples are greater/less than +/-1 they're garbage values, output silence instead.
            if (playbackBuffer != nullptr && fabs((playbackBuffer[channel][uPlaybackSampleHead + localBufferPosition])) <= 1)
            {
                output = playbackBuffer[channel][uPlaybackSampleHead + localBufferPosition];
            }

            // mix generated output with input buffer based on mix value.
            float mixed = (input) * (1 - in_fMix) + output * in_fMix;

            io_paBuffer[channel][uFramesProcessed] = mixed;
        }
        uFramesProcessed++;
        localBufferPosition++;
    }

    uPlaybackSampleHead += uFramesProcessed - 1;
    uCurrentCachedBufferSample += uFramesProcessed - 1;
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
    uBufferSampleSize = splice / (float)1000 * sampleRate; // actual buffer based on splice size
}