#include "splice.h"
#include "math.h"
#include <AK/Tools/Common/AkAssert.h>
#include "buffer_utilities.h"

Splice::Splice() :
	m_bReverse(false),
	m_fSpeed(0.0f),
	m_uSize(0),
	m_uSplice(0),
	//m_uDelay(0.0f),
	//m_fMix(0.0f),
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
	//AkUInt32 uDelay,
	//AkReal32 fMix,
	AkReal32 fRecycle)
{
	m_bReverse = bReverse;
	m_fSpeed = fPitch;
	m_uSplice = uSplice;
	//m_uDelay = uDelay;
	//m_fMix = fMix;
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

AkUInt32 Splice::PushToBuffer(RingBuffer &out_pBuffer, AkUInt16 uCrossfadeFrames)
{
	//const AkReal32 fSpeed = pow(2, m_fPitch / 1200);

	// If speed is less than one we will be interpolating between samples, and the max samples we can interpolate between is m_uSplice - 1
	const AkUInt32 uFramesToWrite = floor(static_cast<AkReal32>(m_uSplice) / m_fSpeed) - (m_fSpeed < 1.0f ? 1 : 0);
	AkUInt32 uFramesWritten = 0;
	AkReal32 fPosition = m_bReverse ? (m_uSplice - 1) : 0.0f;


	const bool bUseFilter = false;

	const AkUInt16 num_taps = 256;
	AkReal32 coefficients[num_taps];
	
	if (bUseFilter && m_fSpeed < 1)
	{
		const AkReal32 pi = 3.14159265358979323846;
		AkReal32 normalized_cutoff = (18000.0f * m_fSpeed) / 48000.0f; // hack

		for (int i = 0; i < num_taps; ++i) {
			if (i - (num_taps - 1) / 2 == 0) {
				coefficients[i] = 2 * normalized_cutoff;
			}
			else {
				coefficients[i] = sin(2 * pi * normalized_cutoff * (i - (num_taps - 1) / 2)) / (pi * (i - (num_taps - 1) / 2));
			}
		}
	}

	const AkInt16 iDirection = m_bReverse ? -1.0f : 1.0f;
	
	while (uFramesWritten < uFramesToWrite)
	{
		AKASSERT(fPosition < m_uSize);
		AKASSERT(fPosition >= 0);

		AkUInt32 uPosition = m_bReverse ? ceil(fPosition) : fPosition;
		AkReal32 fDelta = fabs(fPosition - uPosition);

		AkReal32 curr, next;
		curr = next = m_pData[uPosition];
		
		if ((m_bReverse && fPosition > 0) || (!m_bReverse && fPosition < m_uSplice - 1))
		{
			next = m_pData[uPosition + iDirection];
		}

		AkReal32 fOutput = (1 - fDelta) * curr + (fDelta * next);

		if (bUseFilter && m_fSpeed < 1)
		{
			fOutput *= coefficients[0];
			for (int j = 1; j < num_taps; ++j) {
				fOutput += coefficients[j] * m_pData[uPosition + j];
			}
		}

		if (uFramesWritten < uCrossfadeFrames)
		{
			const float weight = uFramesWritten / uCrossfadeFrames;
			fOutput = CalculateWetDryMix(out_pBuffer.m_pData[out_pBuffer.m_uWritePosition], fOutput, weight);
		}
		else if (uFramesToWrite - uFramesWritten < uCrossfadeFrames)
		{
			const float weight = (uFramesToWrite - uFramesWritten) / uCrossfadeFrames;
			fOutput = CalculateWetDryMix(out_pBuffer.m_pData[out_pBuffer.m_uWritePosition], fOutput, weight);
		}

		out_pBuffer.m_pData[out_pBuffer.m_uWritePosition] = fOutput;
		++out_pBuffer.m_uWritePosition %= out_pBuffer.m_uSize;

		fPosition += m_fSpeed * iDirection;
		uFramesWritten++;
	}

	return uFramesWritten;
}
