#include "MonoBuffer.h"

BoundedMonoBuffer::BoundedMonoBuffer()
{
}

BoundedMonoBuffer::~BoundedMonoBuffer()
{
}

float BoundedMonoBuffer::GetNextBufferValue()
{
	return 0;
}

float* BoundedMonoBuffer::GetNextBufferBlock(const uint& in_uBlockSize)
{
	return new float[in_uBlockSize];
}

bool BoundedMonoBuffer::WriteNextBufferValue(const float& in_fValue)
{
	if (m_bIsFilled || m_uBufferWritePosition >= m_uBufferSize) return false;

	m_pBuffer[m_uBufferWritePosition] = in_fValue;
	m_uBufferWritePosition++;
	if (m_uBufferWritePosition == m_uBufferSize)
	{
		m_bIsFilled = true;
	}

	return false;
}

bool BoundedMonoBuffer::WriteNextBufferBlock(const float* in_pBlock, const uint& in_uBlockSize)
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

bool BoundedMonoBuffer::WriteSilenceBlock(const uint& in_uBlockSize)
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

