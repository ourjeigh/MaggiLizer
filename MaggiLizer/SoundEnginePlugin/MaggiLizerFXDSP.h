#pragma once


class MaggiLizerFXDSP
{
    typedef unsigned int uint;

public:
	MaggiLizerFXDSP();
	~MaggiLizerFXDSP();
	void Init(uint in_uSampleRate, float in_fSplice, uint in_numChannels);
	void Term();
	void Reset();
    void Execute(
        float** io_pBuffer,
        const uint in_uNumChannels,
        const uint in_uValidFrames,
        const bool reverse,
        const float pitch,
        const float splice,
        const float delay,
        const float recycle,
        const float mix);

    void ProcessSingleFrame(
        float** io_pBuffer, 
        float** in_pCachedBuffer,
        float** in_pPlaybackBuffer, 
        const uint channel,
        const uint uFramesProcessed,
        bool& bufferFilled, 
        const float speed,
        const bool in_bReverse,
        const float in_fMix);

    void SetBufferValue(float** io_pBuffer, const uint& in_uChannel, const uint in_uBufferSamplePosition, const float in_fInput);
    float GetBufferValue(float** in_pBuffer, const uint& channel, const uint& in_uBufferPosition);
    float MixInputWithOutput(const float in_fInput, const float in_fOutput, const float in_fMix);
    float CalculateSpeed(float in_fPitch) const;
    void ApplyReverse(float* io_pBuffer, uint in_uBufferSize, bool in_bReverse);
    void ApplySpeed(float* in_pBuffer, float* out_pBuffer, uint in_pBufferSize, float in_fSpeed);

private:
    uint m_uSampleRate;
    uint m_uBufferSampleSize;
    uint m_uCurrentCachedBufferSample;
    uint m_uPlaybackSampleHead;
    float** m_pCachedBuffer;
    float** m_pPlaybackBuffer;

    void ClearBuffer(float* buffer, uint bufferSize);
    void CaclulateBufferSampleSize(float splice);
    void SwapBufferValues(float* a, float* b);
    void ReverseBuffer(float* array, uint array_size);
};

