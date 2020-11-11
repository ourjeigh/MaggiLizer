#pragma once


class MaggiLizerFXDSP
{
    typedef unsigned int uint;
    typedef float** buffer; // multichannel buffer
    typedef float* buffer_single; //single channel buffer

public:
	MaggiLizerFXDSP();
	~MaggiLizerFXDSP();

	void Init(uint in_uSampleRate, float in_fSplice, uint in_numChannels);
	void Term();
	void Reset();
    void Execute
    (
        buffer io_pBuffer,
        const uint in_uNumChannels,
        const uint in_uValidFrames,
        const bool reverse,
        const float pitch,
        const float splice,
        const float delay,
        const float recycle,
        const float mix
    );

    void ProcessSingleFrame
    (
        buffer io_pBuffer,
        buffer in_pCachedBuffer,
        buffer in_pPlaybackBuffer,
        const uint& channel,
        const uint& uFramesProcessed,
        const float& speed,
        const bool& in_bReverse,
        const float& in_fMix,
        bool& bufferFilled
    );

    void SetBufferValue(buffer io_pBuffer, const uint& in_uChannel, const uint in_uBufferSamplePosition, const float in_fInput);
    void SwapBufferValues(float* a, float* b);
    void ClearBufferSingle(buffer_single buffer, const uint bufferSize);
    void ApplyReverseBufferSingle(buffer_single io_pBuffer, uint in_uBufferSize, bool in_bReverse);
    void ApplySpeedBufferSingle(buffer_single in_pBuffer, buffer_single out_pBuffer, const uint& in_uBufferSize, const float& in_fSpeed);

    uint CaclulateBufferSampleSize(const uint& in_uSampleRate, const float& in_fSplice) const;
    uint CalculateBufferSizeChangeFromSpeed(const uint& in_uBufferSize, const float in_fSpeed) const;

    float GetBufferValue(buffer in_pBuffer, const uint& channel, const uint& in_uBufferPosition) const;
    float CalculateWetDryMix(const float in_fDry, const float in_fWet, const float in_fMix) const;
    float CalculateSpeed(float in_fPitch) const;

private:
    uint m_uSampleRate;
    uint m_uBufferSampleSize;
    uint m_uCurrentCachedBufferSample;
    uint m_uPlaybackSampleHead;
    buffer m_pCachedBuffer;
    buffer m_pPlaybackBuffer;
};

