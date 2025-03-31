#include "ring_buffer.h"
#include <AK/Tools/Common/AkPlatformFuncs.h>

// ------------------ RING BUFFER

void RingBuffer::WriteBlock(const AkReal32* in_pData, const AkUInt32 in_uSize)
{
	AKASSERT(in_pData);
	AKASSERT(in_uSize);

	// Make sure we have enough room to write the full input
	AKASSERT((m_uReadPosition - m_uWritePosition + m_uSize - 1) % m_uSize > in_uSize);

	AkUInt32 uSpace = m_uSize - m_uWritePosition;
	AkUInt32 ufirstBlockSize = AkMin(uSpace, in_uSize);

	AKPLATFORM::AkMemCpy(&m_pData[m_uWritePosition], in_pData, sizeof(AkReal32) * ufirstBlockSize);

	AkUInt32 uSecondBlockSize = in_uSize - ufirstBlockSize;
	AKASSERT(uSecondBlockSize >= 0);

	if (uSecondBlockSize)
	{
		// loop back to beginning
		AKPLATFORM::AkMemCpy(m_pData, &in_pData[ufirstBlockSize], sizeof(AkReal32) * uSecondBlockSize);
		m_uWritePosition = uSecondBlockSize;
	}
	else
	{
		m_uWritePosition += ufirstBlockSize;
	}

	AKASSERT(m_uWritePosition < m_uSize);
}

void RingBuffer::WriteSilentBlock(const AkUInt32 in_uSize)
{
	AKASSERT(in_uSize);

	AkUInt32 uSpace = m_uSize - m_uWritePosition;
	AkUInt32 ufirstBlockSize = AkMin(uSpace, in_uSize);

	AkZeroMemLarge(&m_pData[m_uWritePosition], sizeof(AkReal32) * ufirstBlockSize);

	AkUInt32 uSecondBlockSize = in_uSize - ufirstBlockSize;
	AKASSERT(uSecondBlockSize >= 0);

	if (uSecondBlockSize)
	{
		// loop back to beginning
		AkZeroMemLarge(m_pData, sizeof(AkReal32) * uSecondBlockSize);
		m_uWritePosition = uSecondBlockSize;
	}
	else
	{
		m_uWritePosition += ufirstBlockSize;
	}

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
	AkUInt32 uSizeRead = ReadBlockInternal(out_pData, in_uSize, m_uReadPosition);

	// If we didn't read the full block, that means we read all the readable samples.
	// Make sure read and write positions are equal so that HasData will return false.
	AKASSERT(uSizeRead == in_uSize || m_uReadPosition == m_uWritePosition);
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

	AkUInt32 uSpace = m_uSize - inout_uReadPosition - 1;
	AkUInt32 uReadWriteDistance = (m_uWritePosition - m_uReadPosition + m_uSize) % m_uSize;
	AkUInt32 ufirstBlockSize = AkMin(AkMin(uSpace, in_uSize), uReadWriteDistance);

	AKPLATFORM::AkMemCpy(out_pData, &m_pData[inout_uReadPosition], sizeof(AkReal32) * ufirstBlockSize);

	AkUInt32 uSecondBlockSize = AkMin(in_uSize - ufirstBlockSize, uReadWriteDistance - ufirstBlockSize);
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

	AKASSERT(inout_uReadPosition < m_uSize);
	return ufirstBlockSize + uSecondBlockSize;
}