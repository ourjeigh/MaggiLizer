#pragma once

#include "MonoBuffer.h"
#include <math.h>

static class MonoBufferUtilities
{
    typedef unsigned int uint;

private:
    void SwapBufferValues(float* a, float* b)
    {
        float temp = *a;
        *a = *b;
        *b = temp;
    }

public:

    void ApplyReverseBufferSingle(MonoBuffer* io_pBuffer)
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

    void ApplySpeedBufferSingle(float* in_pBuffer, float* out_pBuffer, const uint& in_uBufferSize, const float& in_fSpeed)
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
}