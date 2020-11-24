#include "MonoBuffer.h"

CircularMonoBuffer::CircularMonoBuffer()
{
}

CircularMonoBuffer::~CircularMonoBuffer()
{
}

float CircularMonoBuffer::GetNextBufferValue()
{
	const float out = m_pBuffer[m_uBufferReadPosition];
	m_uBufferReadPosition++;
	m_uBufferReadPosition %= m_uBufferSize;
	return out;
}

float* CircularMonoBuffer::GetNextBufferBlock(const uint& in_uBlockSize)
{
	float* out = new float[in_uBlockSize];
	for (uint i = 0; i < in_uBlockSize; i++)
	{
		out[i] = GetNextBufferValue();
	}
	return out;
}

bool CircularMonoBuffer::WriteNextBufferValue(const float& in_fValue)
{
	m_pBuffer[m_uBufferWritePosition] = in_fValue;
	m_uBufferWritePosition++;
	if (m_uBufferWritePosition == m_uBufferSize)
	{
		m_bIsFilled = true;
		m_uBufferWritePosition = 0;
	}
	return true;
}

bool CircularMonoBuffer::WriteNextBufferBlock(const float* in_pBlock, const uint& in_uBlockSize)
{
	for (int i = 0; i < in_uBlockSize; i++)
	{
		WriteNextBufferValue(in_pBlock[i]);
	}
	return true;
}

bool CircularMonoBuffer::WriteSilenceBlock(const uint& in_uBlockSize)
{
	for (int i = 0; i < in_uBlockSize; i++)
	{
		WriteNextBufferValue(0);
	}
	return true;
}