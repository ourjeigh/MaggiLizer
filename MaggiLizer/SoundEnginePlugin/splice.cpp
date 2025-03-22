#include "splice.h"
#include "math.h"
#include <AK/Tools/Common/AkAssert.h>
#include "buffer_utilies.h"

Splice::Splice() :
	m_bReverse(false),
	m_fPitch(0.0f),
	m_uSize(0),
	m_uDelay(0.0f),
	m_fMix(0.0f),
	m_fRecycle(0.0f),
	m_uWritePosition(0),
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
	AkUInt32 uDelay,
	AkReal32 fMix,
	AkReal32 fRecycle)
{
	m_bReverse = bReverse;
	m_fPitch = fPitch;
	m_uSplice = uSplice;
	m_uDelay = uDelay;
	m_fMix = fMix;
	m_fRecycle = fRecycle;
}

void Splice::MixInBlock(AkReal32* in_pBuffer, AkUInt32 in_uSize)
{
	AkUInt32 uSamplesProcessed = 0;
	while (uSamplesProcessed <= in_uSize && m_uWritePosition < m_uSplice)
	{
		m_pData[m_uWritePosition] = in_pBuffer[uSamplesProcessed] + (m_fRecycle * m_pData[m_uWritePosition]);
		m_uWritePosition++;
		uSamplesProcessed++;
	}
}

AkUInt32 Splice::PushToBuffer(RingBuffer* out_pBuffer)
{
	const AkUInt32 uSmoothWindowSize = 128;
	const AkReal32 fSpeed = pow(2, m_fPitch / 1200);
	const AkUInt32 uFramesToWrite = floor(static_cast<float>(m_uSplice) / fSpeed);
	AkUInt32 uFramesWritten = 0;
	AkReal32 fPosition = 0.0f;

	// doesn't work for reverse yet!
	while (uFramesWritten < uFramesToWrite && fPosition < m_uSplice - 1)
	{
		AKASSERT(fPosition < m_uSize);

		AkUInt32 uPosition = fPosition;
		AkReal32 fDelta = fPosition - uPosition;

		AkReal32 curr = m_pData[uPosition];
		AkReal32 next = m_pData[uPosition + 1];

		AkReal32 fOutput = (1 - fDelta) * curr + (fDelta * next);

		if (uFramesWritten < uSmoothWindowSize)
		{
			const float weight = uFramesWritten / uSmoothWindowSize;
			fOutput = MonoBufferUtilities::CalculateWetDryMix(out_pBuffer->m_pData[out_pBuffer->m_uWritePosition], fOutput, weight);
		}
		else if (uFramesToWrite - uFramesWritten < uSmoothWindowSize)
		{
			const float weight = (uFramesToWrite - uFramesWritten) / uSmoothWindowSize;
			fOutput = MonoBufferUtilities::CalculateWetDryMix(out_pBuffer->m_pData[out_pBuffer->m_uWritePosition], fOutput, weight);
		}

		out_pBuffer->m_pData[out_pBuffer->m_uWritePosition] = fOutput;
		++out_pBuffer->m_uWritePosition %= out_pBuffer->m_uSize;

		fPosition += fSpeed;
		uFramesWritten++;
	}

	return uFramesWritten;
}
