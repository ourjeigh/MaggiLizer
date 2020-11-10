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

    unsigned int uFramesProcessed = 0;

    // Pitch change is just playback speed
    float speed = pow(2, in_fPitch / 1200);


    for (int curCh = 0; curCh < in_uNumChannels; ++curCh)
    {
        uFramesProcessed = 0;

        int localBufferPosition = 0;
        while (uFramesProcessed < in_uValidFrames)
        {
            float input = io_paBuffer[curCh][uFramesProcessed];

            // if cachedBuffer is full move it to playbackBuffer and clear
            // TODO: Need to figure out how to have this apply to all channels after trackers have been reset.
            // -- issue: https://github.com/rjmattingly/MaggiLizer/projects/1#card-49019522
            if (uCurrentCachedBufferSample + localBufferPosition >= uBufferSampleSize)
            {
                ClearBuffer(playbackBuffer[curCh], 4 * sampleRate);
                ApplySpeedAndReverse(cachedBuffer[curCh], playbackBuffer[curCh], uBufferSampleSize, speed, in_bReverse);
                ClearBuffer(cachedBuffer[curCh], 2 * sampleRate);
                uCurrentCachedBufferSample = 0;
                uPlaybackSampleHead = 0;
                localBufferPosition = 0;
            }

            cachedBuffer[curCh][uCurrentCachedBufferSample + localBufferPosition] = input;

            float output = 0;

            // if the playbackBuffer samples are greater/less than +/-1 they're garbage values, output silence instead.
            if (playbackBuffer != nullptr && fabs((playbackBuffer[curCh][uPlaybackSampleHead + localBufferPosition])) <= 1)
            {
                output = playbackBuffer[curCh][uPlaybackSampleHead + localBufferPosition];
            }

            // mix generated output with input buffer based on mix value.
            float mixed = (input) * (1 - in_fMix) + output * in_fMix;

            io_paBuffer[curCh][uFramesProcessed] = mixed;

            uFramesProcessed++;
            localBufferPosition++;
        }
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