#include "MonoBuffer.h"

LinearMonoBuffer::LinearMonoBuffer()
{
}

LinearMonoBuffer::~LinearMonoBuffer()
{
}

/// <summary>
/// Each call to ReadBufferValue increments the read head by one.
/// </summary>
bool LinearMonoBuffer::ReadNextBufferValue(float& out_fValue)
{
	if (m_bReadComplete || m_uBufferReadPosition >= m_uBufferSize)
	{
		m_bReadComplete = true;
		return false;
	}

	out_fValue = m_pBuffer[m_uBufferReadPosition];
	m_uBufferReadPosition++;
	
	return true;
}

/// <summary>
/// Each call to ReadNextBufferBlock increments the write head by in_uBlockSize
/// </summary>
bool LinearMonoBuffer::ReadNextBufferBlock(const uint& in_uBlockSize, float* out_pBuffer)
{
	if (m_bReadComplete || m_uBufferReadPosition + in_uBlockSize >= m_uBufferSize)
	{
		return false;
	}

	out_pBuffer = new float[in_uBlockSize];
	for (uint i = 0; i < in_uBlockSize; i++)
	{
		if (!ReadNextBufferValue(out_pBuffer[i]))
		{
			return false;
		}
	}

	return true;
}

/// <summary>
/// Get upcoming buffer values without incrementing the read head.
/// </summary>
bool LinearMonoBuffer::GetBufferBlock(const uint& in_uBlockSize, float* out_pBuffer)
{
	if (m_bReadComplete || m_uBufferReadPosition + in_uBlockSize >= m_uBufferSize)
	{
		return false;
	}

	out_pBuffer = new float[in_uBlockSize];
	for (uint i = 0; i < in_uBlockSize; i++)
	{
		out_pBuffer[i] = m_pBuffer[m_uBufferReadPosition + i];
	}
	return true;
}

/// <summary>
/// Each call to WriteNextBufferValue incrememnts the write head.
/// </summary>
bool LinearMonoBuffer::WriteNextBufferValue(const float& in_fValue)
{
	if (m_bIsFilled || m_uBufferWritePosition >= m_uBufferSize) return false;

	m_pBuffer[m_uBufferWritePosition] = in_fValue;
	m_uBufferWritePosition++;
	m_bHasData = true;

	if (m_uBufferWritePosition == m_uBufferSize)
	{
		m_bIsFilled = true;
	}

	return true;
}

/// <summary>
/// Each call to WriteNextBufferBlock incrememnts the write head by in_uBlockSize.
/// </summary>
bool LinearMonoBuffer::WriteNextBufferBlock(const float* in_pBlock, const uint& in_uBlockSize)
{
	for (uint i = 0; i < in_uBlockSize; i++)
	{
		if (!WriteNextBufferValue(in_pBlock[i]))
		{
			return false;
		}
	}

	return true;
}

/// <summary>
/// Fill upcoming values with 0s, incrememnting the write head by in_uBlockSize.
/// </summary>
bool LinearMonoBuffer::WriteSilenceBlock(const uint& in_uBlockSize)
{
	for (uint i = 0; i < in_uBlockSize; i++)
	{
		if (!WriteNextBufferValue(0))
		{
			return false;
		}
	}

	return true;
}

bool LinearMonoBuffer::GetLastWrittenBufferBlock(const uint& in_uBlockSize, float* out_pBuffer)
{
	return false;
}

