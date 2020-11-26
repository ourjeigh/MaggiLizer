#pragma once

#include "MonoBuffer.h"
#include <math.h>

static class MonoBufferUtilities
{
    typedef unsigned int uint;

private:
    static void SwapBufferValues(float* a, float* b)
    {
        float temp = *a;
        *a = *b;
        *b = temp;
    }

public:

    /// <summary>
    /// Actual buffer size for iteration is based on splice value
    /// </summary>
    static uint ConvertMillisecondsToSamples(const uint& in_uSampleRate, const float& in_fMilliseconds) 
    {
        // in_fSplice is in milliseconds, divide by 1000 to get seconds.
        return (uint)(in_fMilliseconds / (float)1000 * in_uSampleRate);
    }

    /// <summary>
    /// Pitch change is just playback speed
    /// </summary>
    static float CalculateSpeed(const float& in_fPitch) 
    {
        return pow(2, in_fPitch / 1200);
    }

    static uint CalculateBufferSizeChangeFromSpeed(const uint& in_uBufferSize, const float& in_fSpeed)
    {
        return floor((float)in_uBufferSize / in_fSpeed);
    }

    static void ApplyReverseBufferSingle(MonoBuffer* io_pBuffer)
    {
        float* pointerLeft = io_pBuffer->m_pBuffer;
        float* pointerRight = io_pBuffer->m_pBuffer + io_pBuffer->m_uBufferSize - 1;

        while (pointerLeft < pointerRight)
        {
            SwapBufferValues(pointerLeft, pointerRight);
            pointerLeft++;
            pointerRight--;
        }
    }

    static uint ApplySpeedBufferSingle(MonoBuffer* in_pBuffer, MonoBuffer* out_pBuffer, const float& in_fSpeed)
    {
        uint uOutWriteSize = CalculateBufferSizeChangeFromSpeed(in_pBuffer->m_uBufferSize, in_fSpeed);

        float position = 0.0;
        for (uint i = 0; i < uOutWriteSize; i++)
        {
            // can't go past in_uBufferSize since that's the length of the input buffer.
            if (position >= in_pBuffer->m_uBufferSize - 1)
            {
                // If we were to set position = 0 here it would start repeating the input signal for speeds > 1.
                // This doesn't sound very good, so we just leave the rest of the buffer empty.
                return i;
            }

            uint intPos = floor(position);
            float floatPos = position - intPos;

            float prev = in_pBuffer->m_pBuffer[intPos];
            float next = in_pBuffer->m_pBuffer[intPos + 1];

            out_pBuffer->WriteNextBufferValue((1 - floatPos) * prev + (floatPos * next));

            position += in_fSpeed;
        }
        return uOutWriteSize;
    }

    /// <summary>
    /// out_pBuffer size MUST be at least in_uBufferSize or larger
    /// </summary>
    static bool CopyMonoBuffer(MonoBuffer* in_pBuffer, MonoBuffer* out_pBuffer, const uint& in_uBufferSize)
    {
        if (out_pBuffer->m_uBufferSize < in_uBufferSize) return false;

        return out_pBuffer->PrefillBuffer(in_pBuffer->m_pBuffer, in_uBufferSize);
    }

    static bool CopyLastWrittenBufferBlock(MonoBuffer* in_pBuffer, MonoBuffer* out_pBuffer, const uint& in_uBlockSize)
    {
        float* pBufferBlock =  new float[in_uBlockSize];
        in_pBuffer->GetLastWrittenBufferBlock(in_uBlockSize, pBufferBlock);
        out_pBuffer->PrefillBuffer(pBufferBlock, in_uBlockSize);
        return true;
    }

    static float CalculateWetDryMix(const float& in_fDry, const float& in_fWet, const float in_fMix) 
    {
        return (in_fDry) * (1 - in_fMix) + in_fWet * in_fMix;
    }

    static float ReadBufferMixWithDry(MonoBuffer* io_pBuffer, const float& in_fDry, const float& in_fMix)
    {
        float fWet = 0;
        io_pBuffer->ReadNextBufferValue(fWet);

        return CalculateWetDryMix(in_fDry, fWet, in_fMix);
    }

    static void ApplySmoothingAtIndex(MonoBuffer* io_pBuffer, const uint& in_uPosition, const uint& in_uSmoothWindowSize)
    {
        const float fStartValue = io_pBuffer->m_pBuffer[in_uPosition];
        for (uint i = in_uPosition; i < in_uPosition + in_uSmoothWindowSize; i++)
        {
            const float weight = 1.0 - ((float)i / (float)in_uSmoothWindowSize);
            const float smoothedValue = MonoBufferUtilities::CalculateWetDryMix(io_pBuffer->m_pBuffer[i], fStartValue, weight);
            io_pBuffer->m_pBuffer[i] = smoothedValue;
        }
    }
};