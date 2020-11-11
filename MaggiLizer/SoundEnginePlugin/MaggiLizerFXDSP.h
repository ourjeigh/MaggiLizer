#pragma once
class MaggiLizerFXDSP
{
public:
	MaggiLizerFXDSP();
	~MaggiLizerFXDSP();
	void Init(int in_uSampleRate, float in_fSplice, int in_numChannels);
	void Term();
	void Reset();
    void Execute(
        float** io_paBuffer,
        const int in_uNumChannels,
        const int in_uValidFrames,
        const bool reverse,
        const float pitch,
        const float splice,
        const float delay,
        const float recycle,
        const float mix);

    void ProcessSingleFrame(
        float** io_paBuffer, 
        float** in_pCachedBuffer,
        float** in_pPlaybackBuffer, 
        const int channel,
        const unsigned int uFramesProcessed,
        bool& bufferFilled, 
        const float speed,
        const bool in_bReverse,
        const float in_fMix);

    float GetBufferValue(float** in_pPlaybackBuffer, const int& channel, const unsigned int& in_playbackBufferHead);

    float MixInputWithOutput(const float in_fInput, const float in_fOutput, const float in_fMix);
    float CalculateSpeed(float in_fPitch) const;

private:
    unsigned int m_uSampleRate;
    unsigned int m_uBufferSampleSize;
    unsigned int m_uCurrentCachedBufferSample;
    unsigned int m_uPlaybackSampleHead;
    float** m_pCachedBuffer;
    float** m_pPlaybackBuffer;

    void ApplySpeedAndReverse(float* inBuffer, float* outBuffer, int bufferSize, float speed, bool b_reverse);
    void ClearBuffer(float* buffer, int bufferSize);
    void CaclulateBufferSampleSize(float splice);
};

