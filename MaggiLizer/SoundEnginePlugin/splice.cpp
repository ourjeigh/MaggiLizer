#include "splice.h"
#include "math.h"
#include <AK/Tools/Common/AkAssert.h>
#include "utilities.h"
#include <AK/Tools/Win32/AkPlatformFuncs.h>

Splice::Splice() :
	m_bReverse(false),
	m_fSpeed(0.0f),
	m_uSize(0),
	m_uSplice(0),
	m_fRecycle(0.0f),
	m_uWritePosition(0),
	m_uOutputReadPosition(0),
	m_pData(nullptr)
{
}

Splice::~Splice()
{
}

void Splice::AttachData(AkReal32* in_pData, AkUInt32 in_uSize)
{
	m_pData = in_pData;
	m_uSize = in_uSize;
}

void Splice::UpdateSettings(
	bool bReverse,
	AkReal32 fPitch,
	AkUInt32 uSplice,
	AkReal32 fRecycle)
{
	m_bReverse = bReverse;
	m_fSpeed = fPitch;
	m_uSplice = uSplice;
	m_fRecycle = fRecycle;
}

void Splice::MixInBlock(AkReal32* in_pInputBuffer, AkReal32* in_pRecycleBuffer, AkUInt32 in_uSize)
{
	AkUInt32 uSamplesProcessed = 0;
	//while (uSamplesProcessed <= in_uSize && m_uWritePosition < m_uSplice)
	while (uSamplesProcessed < in_uSize && m_uWritePosition < m_uSplice)
	{
		// todo: make this simd
		AkReal32 fInput = in_pInputBuffer[uSamplesProcessed];
		AkReal32 fRecycle = in_pRecycleBuffer[uSamplesProcessed];
		AkReal32 fWrite = fInput + (m_fRecycle * fRecycle);

		ASSERT_VOLUME(fInput);
		ASSERT_VOLUME(fRecycle);
		ASSERT_VOLUME(fWrite);

		m_pData[m_uWritePosition++] = fWrite;
		uSamplesProcessed++;
	}
}

AkUInt32 Splice::PushToBuffer(RingBuffer& out_pBuffer, AkUInt16 uCrossfadeFrames)
{
	// If speed is less than one we would generate more samples than we have in the splice buffer
	const AkUInt32 uFramesToWrite = AkMin(static_cast<AkReal32>(m_uSplice) / m_fSpeed, m_uSplice);
	AkUInt32 uFramesWritten = 0;
	AkReal32 fReadPosition = m_bReverse ? (m_uSplice - 1) : 0.0f;

	const AkInt16 iDirection = m_bReverse ? -1.0f : 1.0f;

	while (uFramesWritten < uFramesToWrite)
	{
		AKASSERT(fReadPosition < m_uSize);
		AKASSERT(fReadPosition >= 0);

		AkUInt32 uReadPosition = m_bReverse ? ceil(fReadPosition) : fReadPosition;
		AkReal32 fDelta = fabs(fReadPosition - uReadPosition);

		AkReal32 curr, next;
		curr = next = m_pData[uReadPosition];

		if ((m_bReverse && fReadPosition > 0) || (!m_bReverse && fReadPosition < m_uSplice - 1))
		{
			next = m_pData[uReadPosition + iDirection];
		}

		AkReal32 fOutput = (1 - fDelta) * curr + (fDelta * next);

		if (uFramesWritten < uCrossfadeFrames)
		{
			AkReal32 weight = static_cast<AkReal32>(uFramesWritten) / uCrossfadeFrames;
			fOutput = CalculateWetDryMix(out_pBuffer.m_pData[out_pBuffer.m_uWritePosition], fOutput, weight);
		}
		else if (uFramesToWrite - uFramesWritten <= uCrossfadeFrames)
		{
			AkReal32 weight = static_cast<AkReal32>(uFramesToWrite - uFramesWritten - 1) / uCrossfadeFrames;
			fOutput = CalculateWetDryMix(out_pBuffer.m_pData[out_pBuffer.m_uWritePosition], fOutput, weight);
		}

		ASSERT_VOLUME(fOutput);

		out_pBuffer.m_pData[out_pBuffer.m_uWritePosition] = fOutput;
		++out_pBuffer.m_uWritePosition %= out_pBuffer.m_uSize;

		fReadPosition += m_fSpeed * iDirection;
		uFramesWritten++;
	}

	return uFramesWritten;
}