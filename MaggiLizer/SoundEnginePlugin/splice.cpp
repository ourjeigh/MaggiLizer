#include <AK/Tools/Common/AkAssert.h>
#include "splice.h"
#include "utilities.h"
using namespace AKPLATFORM;

#define UINT32_MAX 0xffffffff


Splice::Splice() :
	m_uAttachedBufferSize(0)
{
	Reset();
}

Splice::~Splice()
{
}

void Splice::Reset()
{
	m_Settings.Clear();

	m_uReadPosition = 0;
	m_uEndPosition = 0;
	m_uDelaySamplesRemaining = 0;
}

void Splice::PrepareNextSplice(const SpliceSettings& settings)
{
	AKASSERT(settings.uSpliceSamples);

	m_Settings.Clear();
	m_Settings = settings;

	if (m_Settings.bReverse)
	{
		m_uEndPosition = m_uReadPosition;
		m_uReadPosition = (m_uEndPosition + m_Settings.uSpliceSamples) % m_uAttachedBufferSize;
	}
	else
	{
		m_uEndPosition = (m_uReadPosition + m_Settings.uSpliceSamples) % m_uAttachedBufferSize;
	}
}


void Splice::Process(
	RingBuffer* pBuffer,
	AkUInt32 uSize,
	AkReal32* out_pBuffer,
	AkReal32* out_pRecycleBuffer)
{
	AKASSERT(pBuffer);
	AKASSERT(out_pBuffer);
	AKASSERT(uSize);

	pBuffer->PeekBlock(out_pBuffer, uSize, m_uReadPosition); // temp hack

	RecycleBufferBIntoA(out_pRecycleBuffer, out_pBuffer, uSize, m_Settings.fRecycle);

	m_uReadPosition = (m_uReadPosition + uSize) % m_uAttachedBufferSize;
}

#if 0 // old
AkUInt32 Splice::PushToBuffer(RingBuffer& out_pBuffer, bool bApplySmoothing)
{
	// If speed is less than one we would generate more samples than we have in the splice buffer.
	// This creates a lot of problems with creating faster than we consume, so just clamp it to splice size
	// This would be better if this was pitch in time.
	const AkUInt32 uFramesToWrite = m_fSpeed > 1.0f ? static_cast<AkReal32>(m_uSplice) / m_fSpeed : m_uSplice;
	AkUInt32 uFramesWritten = 0;

	AkUInt32 &uWritePosition = out_pBuffer.m_uWritePosition;

	// we need extra precision for large buffers
	AkReal64 fReadPosition = m_bReverse ? (m_uSplice - 1) : 0.0f;

	const AkInt16 iDirection = m_bReverse ? -1.0f : 1.0f;

	AKASSERT(uFramesToWrite <= m_uSplice);
	AKASSERT(floor(uFramesToWrite * m_fSpeed) <= m_uSplice);

	// In the case of pitching up (speed > 1), we will not create enough samples to fill the splice buffer.
	// For now we'll just fill the rest with silence so that we're always pushing at the same rate as incoming data
	while (uFramesWritten < m_uSplice)
	{
		AkReal32 fOutput = 0.0f;

		if ((uFramesWritten < uFramesToWrite))
		{
			AKASSERT(m_bReverse || fReadPosition <= m_uSplice);
			AKASSERT(!m_bReverse || fReadPosition > -1.0f);
			
			AkUInt32 uReadPosition = m_bReverse ? ceil(fReadPosition) : fReadPosition;
			AKASSERT(uReadPosition < m_uSplice);

			AkReal32 fDelta = fabs(fReadPosition - uReadPosition);

			AkReal32 curr, next;
			curr = next = m_pData[uReadPosition];

			if ((m_bReverse && fReadPosition > 0) || (!m_bReverse && fReadPosition < m_uSplice - 1))
			{
				next = m_pData[uReadPosition + iDirection];
			}

			fOutput = (1 - fDelta) * curr + (fDelta * next);
			fReadPosition += m_fSpeed * iDirection;
		}
		

		AkUInt32 fFramesLeftToWrite = uFramesToWrite - uFramesWritten;
		if (bApplySmoothing && uFramesWritten < m_uSmoothingFrames)
		{
			//AkReal32 fOutputSmoothed = CalculateEqualPowerFadeIn(uFramesWritten, m_uSmoothingFrames, fOutput);
			AkReal32 fOutputSmoothed= CalculateEqualPowerXfade(uFramesWritten, m_uSmoothingFrames, out_pBuffer.m_pData[uWritePosition], fOutput);
			fOutput = fOutputSmoothed;
			/*AkReal32 weight = static_cast<AkReal32>(uFramesWritten) / m_uSmoothingFrames;
			fOutput = CalculateWetDryMix(out_pBuffer.m_pData[out_pBuffer.m_uWritePosition], fOutput, weight);*/
		}
		else if (bApplySmoothing && fFramesLeftToWrite <= m_uSmoothingFrames)
		{
			//AkReal32 fOutputSmoothed = CalculateEqualPowerFadeOut(m_uSmoothingFrames - fFramesLeftToWrite + 1, m_uSmoothingFrames, fOutput);
			AkReal32 fOutputSmoothed = CalculateEqualPowerXfade(m_uSmoothingFrames - fFramesLeftToWrite + 1, m_uSmoothingFrames, fOutput, out_pBuffer.m_pData[uWritePosition]);
			fOutput = fOutputSmoothed;
			/*AkReal32 weight = static_cast<AkReal32>(uFramesToWrite - uFramesWritten - 1) / m_uSmoothingFrames;
			fOutput = CalculateWetDryMix(out_pBuffer.m_pData[out_pBuffer.m_uWritePosition], fOutput, weight);*/
		}

		out_pBuffer.m_pData[uWritePosition++] = fOutput;
		uWritePosition %= out_pBuffer.m_uSize;

		//AKASSERT(out_pBuffer.m_uReadPosition != out_pBuffer.m_uWritePosition);

		uFramesWritten++;
	}
	
	//AKASSERT(out_pBuffer.m_uReadPosition != out_pBuffer.m_uWritePosition);
	return uFramesWritten;
}
#endif