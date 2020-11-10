#pragma once
class MaggiLizerFXDSP
{
public:
	MaggiLizerFXDSP();
	~MaggiLizerFXDSP();
	void Init(int in_uSampleRate, float in_fSplice, int in_numChannels);
	void Term();
	void Reset();
    void Execute(float** io_paBuffer,
                 int in_uNumChannels,
                 int in_uValidFrames,
                 bool reverse,
                 float pitch,
                 float splice,
                 float delay,
                 float recycle,
                 float mix);

private:
    int sampleRate;
    int uBufferSampleSize;
    int uCurrentCachedBufferSample;
    int uPlaybackSampleHead;
    float** cachedBuffer;
    float** playbackBuffer;

    void ApplySpeedAndReverse(float* inBuffer, float* outBuffer, int bufferSize, float speed, bool b_reverse);
    void ClearBuffer(float* buffer, int bufferSize);
    void CaclulateBufferSampleSize(float splice);
};

