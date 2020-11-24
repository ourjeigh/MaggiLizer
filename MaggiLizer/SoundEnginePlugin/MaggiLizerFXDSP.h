#pragma once

#include "MonoBuffer.h"


class MaggiLizerFXDSP
{
    typedef unsigned int uint;
    //typedef float** buffer; // multichannel buffer
    //typedef float* buffer_single; //single channel buffer

public:
	MaggiLizerFXDSP();
	~MaggiLizerFXDSP();

	void Init(uint in_uSampleRate, uint in_numChannels);
	void Term();
	void Reset();
    void Execute
    (
        float** io_pBuffer,
        const uint in_uNumChannels,
        const uint in_uValidFrames,
        const bool& reverse,
        const float& pitch,
        const float& splice,
        const float& delay,
        const float& recycle,
        const float& mix
    );

    void ProcessChannel(float* io_pBuffer, MonoBuffer* in_pCachedBuffer, MonoBuffer* in_pPlaybackBuffer, uint in_uBlockSize, bool reverse);

   /* void ProcessSingleFrame
    (
        buffer io_pBuffer,
        buffer in_pCachedBuffer,
        buffer in_pPlaybackBuffer,
        const uint& channel,
        const uint& uFramesProcessed,
        const float& speed,
        const bool& in_bReverse,
        const float& in_fRecycle,
        const float& in_fMix,
        bool& bufferFilled
    );*/


    void SetBufferValue(float** io_pBuffer, const uint& in_uChannel, const uint& in_uBufferSamplePosition, const float& in_fInput);
    void SwapBufferValues(float* a, float* b);
    //void ClearBufferSingle(buffer_single buffer, const uint& bufferSize);
    void CopyBufferSingleValues(float* in_pBuffer, float* out_pBuffer, const uint& in_uBufferSize);
    void ApplyReverseBufferSingle(float* io_pBuffer, const uint& in_uBufferSize, const bool& in_bReverse);
    void ApplySpeedBufferSingle(float* in_pBuffer, float* out_pBuffer, const uint& in_uBufferSize, const float& in_fSpeed);
    void ApplyRecycleBufferValue(float** io_pBuffer, const uint& channel, const uint& samplePosition, const float& input, const float& in_fRecycle);
    void ApplySmoothBufferSingle(float* io_pBuffer, const float& in_fStartValue, const uint& in_uSmoothWindowSize);

    uint ConvertMillisecondsToSamples(const uint& in_uSampleRate, const float& in_fMilliseconds) const;
    uint CalculateBufferSizeChangeFromSpeed(const uint& in_uBufferSize, const float& in_fSpeed) const;

    float GetBufferValue(float** in_pBuffer, const uint& channel, const uint& in_uBufferPosition) const;
    float CalculateWetDryMix(const float& in_fDry, const float& in_fWet, const float in_fMix) const;
    float CalculateSpeed(const float& in_fPitch) const;

private:
    uint m_uSampleRate;
    uint m_uMaxBufferSize;
    uint m_uSpliceSampleSize;
    //uint m_uCurrentCachedBufferSample;
    //uint m_uPlaybackSampleHead;
    MonoBuffer* m_pSpliceBuffer;
    MonoBuffer* m_pPlaybackBuffer;
    static const uint m_cSmoothWindowSize = 30;
};

