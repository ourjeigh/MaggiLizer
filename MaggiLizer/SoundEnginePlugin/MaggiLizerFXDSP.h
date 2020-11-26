#pragma once
#define MaggiLizerFXDSP_H

#include "MonoBuffer.h"


class MaggiLizerFXDSP
{
    typedef unsigned int uint;

public:
	MaggiLizerFXDSP();
	~MaggiLizerFXDSP();

	void Init(uint in_uSampleRate, uint in_numChannels, uint in_fSplice);
	void Term();
	void Reset();
    void Execute
    (
        float** io_pBuffer,
        const uint in_uNumChannels,
        const uint in_uBlockSize,
        const bool& reverse,
        const float& pitch,
        const float& splice,
        const float& delay,
        const float& recycle,
        const float& mix
    );

    void ProcessSingleFrame
    (
        float* io_pBuffer,
        MonoBuffer* io_pSpliceBuffer,
        MonoBuffer* io_pPlaybackBuffer,
        const uint& in_uFrame,
        const bool& in_bReverse,
        const float& in_fPitch,
        const float& in_fSplice,
        const float& in_fDelay,
        const float& in_fRecycle,
        const float& in_fMix
    );

private:
    uint m_uSampleRate;
    LinearMonoBuffer** m_pSpliceBuffer;
    CircularMonoBuffer** m_pPlaybackBuffer;
    static const uint m_cSmoothWindowSize = 30;
    static const uint m_uBufferLengthMilliseconds = 6000;
};

