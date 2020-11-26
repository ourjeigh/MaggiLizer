#include "MonoBuffer.h"

CircularMonoBuffer::CircularMonoBuffer()
{
}

CircularMonoBuffer::~CircularMonoBuffer()
{
}

/// <summary>
/// Each call to ReadBufferValue increments the read head by one.
/// </summary>
bool CircularMonoBuffer::ReadNextBufferValue(float& out_fValue)
{
	uint delayOffset = (m_uBufferReadPosition + m_uBufferSize + m_uBufferReadDelay) % m_uBufferSize;

	out_fValue = m_pBuffer[delayOffset];
	m_uBufferReadPosition++;
	m_uBufferReadPosition %= m_uBufferSize;
	return true;
}

/// <summary>
/// Each call to ReadNextBufferBlock increments the write head by in_uBlockSize
/// </summary>
bool CircularMonoBuffer::ReadNextBufferBlock(const uint& in_uBlockSize, float* out_pBuffer)
{
	out_pBuffer = new float[in_uBlockSize];
	for (uint i = 0; i < in_uBlockSize; i++)
	{
		ReadNextBufferValue(out_pBuffer[i]);
	}
	return true;
}

/// <summary>
/// Get upcoming buffer values without incrementing the read head.
/// </summary>
bool CircularMonoBuffer::GetBufferBlock(const uint& in_uBlockSize, float* out_pBuffer)
{
	float* out = new float[in_uBlockSize];
	uint filled = 0;
	uint readHead = m_uBufferReadPosition;
	while (filled < in_uBlockSize)
	{
		out[filled] = m_pBuffer[readHead];
		readHead++;
		readHead%= m_uBufferSize;
		filled++;
	}
	return true;
}

/// <summary>
/// Each call to WriteNextBufferValue incrememnts the write head.
/// </summary>
bool CircularMonoBuffer::WriteNextBufferValue(const float& in_fValue)
{
	m_pBuffer[m_uBufferWritePosition] = in_fValue;
	m_uBufferWritePosition++;
	m_bHasData = true;

	if (m_uBufferWritePosition == m_uBufferSize)
	{
		m_bIsFilled = true;
		m_uBufferWritePosition = 0;
	}
	return true;
}

/// <summary>
/// Each call to WriteNextBufferBlock incrememnts the write head by in_uBlockSize.
/// </summary>
bool CircularMonoBuffer::WriteNextBufferBlock(const float* in_pBlock, const uint& in_uBlockSize)
{
	for (uint i = 0; i < in_uBlockSize; i++)
	{
		WriteNextBufferValue(in_pBlock[i]);
	}
	return true;
}

/// <summary>
/// Fill upcoming values with 0s, incrememnting the write head by in_uBlockSize.
/// </summary>
bool CircularMonoBuffer::WriteSilenceBlock(const uint& in_uBlockSize)
{
	for (uint i = 0; i < in_uBlockSize; i++)
	{
		WriteNextBufferValue(0);
	}
	return true;
}

//bool CircularMonoBuffer::SetReadDelay(const uint& in_uBlockSize)
//{
//	m_uBufferWritePosition = (m_uBufferWritePosition + in_uBlockSize) % m_uBufferSize;
//	return true;
//}

bool CircularMonoBuffer::GetLastWrittenBufferBlock(const uint& in_uBlockSize, float* out_pBuffer)
{
	uint iStartPosition = m_uBufferWritePosition - in_uBlockSize;
	if (m_uBufferWritePosition + in_uBlockSize < m_uBufferSize)
	{
		iStartPosition = m_uBufferSize - (m_uBufferWritePosition + in_uBlockSize);
	}
	for (uint i = 0; i < in_uBlockSize; i++)
	{
		out_pBuffer[i] = m_pBuffer[iStartPosition];
		iStartPosition++;
		iStartPosition %= m_uBufferSize;
	}
	return false;
}