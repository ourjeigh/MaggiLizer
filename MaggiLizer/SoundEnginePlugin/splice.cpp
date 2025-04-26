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
	m_uRepeatCount = 0;
}

void Splice::PrepareNextSplice(const SpliceSettings& settings)
{
	AKASSERT(settings.uSpliceSamples);
	AKASSERT(IsReady());

	m_Settings.Clear();
	m_Settings = settings;

	if (m_Settings.bReverse)
	{
		m_uEndPosition = (m_Settings.uSpliceSamples * m_uRepeatCount++) % m_uAttachedBufferSize;
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

	if (m_Settings.bReverse)
	{
		AkUInt32 uReverseReadStart = (m_uReadPosition + m_uAttachedBufferSize - uSize) % m_uAttachedBufferSize;
		pBuffer->PeekBlock(out_pBuffer, uSize, uReverseReadStart);

		if (m_Settings.fRecycle > k_float_threshold)
		{
			RecycleBufferBIntoA(out_pRecycleBuffer, out_pBuffer, uSize, m_Settings.fRecycle);
		}

		ReverseBuffer(out_pBuffer, uSize);

		m_uReadPosition = (m_uReadPosition - uSize) % m_uAttachedBufferSize;
	}
	else
	{
		pBuffer->PeekBlock(out_pBuffer, uSize, m_uReadPosition);

		if (m_Settings.fRecycle > k_float_threshold)
		{
			RecycleBufferBIntoA(out_pRecycleBuffer, out_pBuffer, uSize, m_Settings.fRecycle);
		}

		m_uReadPosition = (m_uReadPosition + uSize) % m_uAttachedBufferSize;
	}
}
