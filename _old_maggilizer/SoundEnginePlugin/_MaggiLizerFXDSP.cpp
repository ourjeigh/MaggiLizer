#include "MaggiLizerFXDSP.h"
#include "MonoBufferUtilities.cpp"
#include <math.h>

MaggiLizerFXDSP::MaggiLizerFXDSP() :
    m_pSpliceBuffer(nullptr),
    m_pPlaybackBuffer(nullptr),
    m_uSampleRate(0)
{
}

MaggiLizerFXDSP::~MaggiLizerFXDSP()
{
}

void MaggiLizerFXDSP::Init(uint in_uSampleRate, uint in_numChannels, uint in_fSplice)
{
    m_uSampleRate = in_uSampleRate;
    const uint uMaxBufferSize = m_uBufferLengthMilliseconds/1000 * m_uSampleRate;

    m_pSpliceBuffer = new LinearMonoBuffer*[in_numChannels];
    m_pPlaybackBuffer = new CircularMonoBuffer*[in_numChannels];


    for (uint i = 0; i < in_numChannels; i++)
    {
        const uint uSpliceSampleSize = MonoBufferUtilities::ConvertMillisecondsToSamples(m_uSampleRate, in_fSplice);
        m_pSpliceBuffer[i] = new LinearMonoBuffer(uSpliceSampleSize);
        m_pPlaybackBuffer[i] = new CircularMonoBuffer(uMaxBufferSize);
    }
}

void MaggiLizerFXDSP::Term()
{
}

void MaggiLizerFXDSP::Reset()
{
}

void MaggiLizerFXDSP::Execute(
    float**  io_pBuffer,
    const uint in_uNumChannels,
    const uint in_uBlockSize,
    const bool& in_bReverse,
    const float& in_fPitch,
    const float& in_fSplice,
    const float& in_fDelay,
    const float& in_fRecycle,
    const float& in_fMix)
{
    uint uFramesProcessed = 0;
    bool bBufferFilled = false;

    for (uint channel = 0; channel < in_uNumChannels; channel++)
    {
        for (uint frame = 0; frame < in_uBlockSize; frame++)
        {
            ProcessSingleFrame(
                io_pBuffer[channel],
                m_pSpliceBuffer[channel],
                m_pPlaybackBuffer[channel],
                frame,
                in_bReverse,
                in_fPitch,
                in_fSplice,
                in_fDelay,
                in_fRecycle,
                in_fMix
            );
        }
    }
}

void MaggiLizerFXDSP::ProcessSingleFrame(
        float* io_pBuffer,
        MonoBuffer* io_pSpliceBuffer,
        MonoBuffer* io_pPlaybackBuffer,
        const uint& in_uFrame, 
        const bool& in_bReverse,
        const float & in_fPitch,
        const float& in_fSplice,
        const float& in_fDelay,
        const float& in_fRecycle,
        const float& in_fMix)
{
    float input = io_pBuffer[in_uFrame];
    
    // Delay
    if (in_fDelay > 0)
    {
        const uint delaySampleSize = MonoBufferUtilities::ConvertMillisecondsToSamples(m_uSampleRate, in_fDelay);
        io_pPlaybackBuffer->SetReadDelay(delaySampleSize);
    }

    //Splice
    if (io_pSpliceBuffer->IsFilled())
    {
        // Reverse 
        if (in_bReverse)
        {
            MonoBufferUtilities::ApplyReverseBufferSingle(io_pSpliceBuffer);
        }
        
        // Pitch
        const uint playbackWritePosition = io_pPlaybackBuffer->WritePosition();
        const float fSpeed = MonoBufferUtilities::CalculateSpeed(in_fPitch);
        const uint filledPlaybackSampleSize = MonoBufferUtilities::ApplySpeedBufferSingle(io_pSpliceBuffer, io_pPlaybackBuffer, fSpeed);
        
        //MonoBufferUtilities::ApplySmoothingAtIndex(io_pPlaybackBuffer, playbackWritePosition, m_cSmoothWindowSize);
        const uint uSpliceSampleSize = MonoBufferUtilities::ConvertMillisecondsToSamples(m_uSampleRate, in_fSplice);
        io_pSpliceBuffer->SetBufferSize(uSpliceSampleSize);
        
        //Recyle
        MonoBufferUtilities::CopyLastWrittenBufferBlock(io_pPlaybackBuffer, io_pSpliceBuffer, filledPlaybackSampleSize);
    }

    float recycled = 0;
    io_pSpliceBuffer->ReadNextBufferValue(recycled);
    recycled = input + recycled * in_fRecycle;

    io_pSpliceBuffer->WriteNextBufferValue(recycled);

    float output = 0;
    if (io_pPlaybackBuffer->HasData())
    {
        io_pPlaybackBuffer->ReadNextBufferValue(output);
    }

    float mix = MonoBufferUtilities::CalculateWetDryMix(input, output, in_fMix);

    io_pBuffer[in_uFrame] = mix;
}