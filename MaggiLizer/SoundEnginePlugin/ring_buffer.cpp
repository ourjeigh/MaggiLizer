#include "ring_buffer.h"
#include <AK/Tools/Common/AkPlatformFuncs.h>

// ------------------ RING BUFFER

void RingBuffer::WriteBlock(const AkReal32* in_pData, const AkUInt32 in_uSize)
{
	AKASSERT(in_pData);
	AKASSERT(in_uSize);

	// First copy as many samples as will fit between the write position and the end of the array
	AkUInt32 uSamplesToEnd = m_uSize - m_uWritePosition;
	AkUInt32 ufirstBlockSize = AkMin(uSamplesToEnd, in_uSize);

	AKPLATFORM::AkMemCpy(&m_pData[m_uWritePosition], in_pData, sizeof(AkReal32) * ufirstBlockSize);

	// Then if we didn't copy all the input samples, copy the rest to the beginning of the array
	AkUInt32 uSecondBlockSize = in_uSize - ufirstBlockSize;
	AKASSERT(uSecondBlockSize >= 0);

	if (uSecondBlockSize)
	{
		AKPLATFORM::AkMemCpy(m_pData, &in_pData[ufirstBlockSize], sizeof(AkReal32) * uSecondBlockSize);
		m_uWritePosition = uSecondBlockSize;
	}
	else
	{
		m_uWritePosition += ufirstBlockSize;
	}

	m_uWritePosition %= m_uSize;

	AKASSERT(m_uWritePosition < m_uSize);
}

void RingBuffer::WriteSilentBlock(const AkUInt32 in_uSize)
{
	AKASSERT(in_uSize);

	AkUInt32 uSpace = m_uSize - m_uWritePosition;
	AkUInt32 ufirstBlockSize = AkMin(uSpace, in_uSize);

	// First copy as many samples as will fit between the write position and the end of the array
	AkZeroMemLarge(&m_pData[m_uWritePosition], sizeof(AkReal32) * ufirstBlockSize);

	// Then if we didn't copy all the input samples, copy the rest to the beginning of the array
	AkUInt32 uSecondBlockSize = in_uSize - ufirstBlockSize;
	AKASSERT(uSecondBlockSize >= 0);

	if (uSecondBlockSize)
	{
		AkZeroMemLarge(m_pData, sizeof(AkReal32) * uSecondBlockSize);
		m_uWritePosition = uSecondBlockSize;
	}
	else
	{
		m_uWritePosition += ufirstBlockSize;
	}

	m_uWritePosition %= m_uSize;

	AKASSERT(m_uWritePosition < m_uSize);
}

void RingBuffer::AdvanceWriteHead(const AkUInt32 in_uSize)
{
	m_uWritePosition = (m_uWritePosition + in_uSize) % m_uSize;
	AKASSERT(m_uWritePosition < m_uSize);
}

void RingBuffer::BacktrackWriteHead(const AkUInt32 in_uSize)
{
	m_uWritePosition = (m_uWritePosition - in_uSize + m_uSize) % m_uSize;
	AKASSERT(m_uWritePosition >= 0);
	AKASSERT(m_uWritePosition < m_uSize);
}

void RingBuffer::ReadBlock(AkReal32* out_pData, const AkUInt32 in_uSize)
{
	AKASSERT(HasData());
	AkUInt32 uSizeRead = ReadBlockInternal(out_pData, in_uSize, m_uReadPosition);
}

void RingBuffer::PeekReadBlock(AkReal32* out_pData, const AkUInt32 in_uSize) const
{
	AkUInt32 uReadPosition = m_uReadPosition;
	ReadBlockInternal(out_pData, in_uSize, uReadPosition);
}

AkUInt32 RingBuffer::PeekBlock(AkReal32* out_pData, const AkUInt32 in_uSize, const AkUInt32 in_uReadPosition) const
{
	AkUInt32 out_uReadPosition = in_uReadPosition;
	ReadBlockInternal(out_pData, in_uSize, out_uReadPosition);

	return out_uReadPosition;
}

// private
AkUInt32 RingBuffer::ReadBlockInternal(AkReal32* out_pData, const AkUInt32 in_uSize, AkUInt32& inout_uReadPosition) const
{
	AKASSERT(out_pData);
	AKASSERT(in_uSize);
	AKASSERT(inout_uReadPosition >= 0);
	AKASSERT(inout_uReadPosition < m_uSize);
	//AKASSERT(HasData());

	AkUInt32 uSpace = m_uSize - inout_uReadPosition;

	// This has got to go. We don't care about distance to read position anymore.
	//AkUInt32 uReadWriteDistance = (m_uWritePosition - m_uReadPosition + m_uSize) % m_uSize;
	//AkUInt32 ufirstBlockSize = AkMin(AkMin(uSpace, in_uSize), uReadWriteDistance);
	AkUInt32 ufirstBlockSize = AkMin(uSpace, in_uSize);

	AKPLATFORM::AkMemCpy(out_pData, &m_pData[inout_uReadPosition], sizeof(AkReal32) * ufirstBlockSize);

	//AkUInt32 uSecondBlockSize = AkMin(in_uSize - ufirstBlockSize, uReadWriteDistance - ufirstBlockSize);
	AkUInt32 uSecondBlockSize = in_uSize - ufirstBlockSize;
	AKASSERT(uSecondBlockSize >= 0);

	if (uSecondBlockSize)
	{
		// loop back to beginning
		AKPLATFORM::AkMemCpy(&out_pData[ufirstBlockSize], m_pData, sizeof(AkReal32) * uSecondBlockSize);
		inout_uReadPosition = uSecondBlockSize;
	}
	else
	{
		inout_uReadPosition += ufirstBlockSize;
	}

	inout_uReadPosition %= m_uSize;

	AKASSERT(inout_uReadPosition < m_uSize);
	return ufirstBlockSize + uSecondBlockSize;
}