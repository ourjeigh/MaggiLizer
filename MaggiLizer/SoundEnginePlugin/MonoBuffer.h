#pragma once

class MonoBuffer
{
public:
	typedef unsigned int uint;
	friend class MonoBufferUtilities;

	virtual float GetNextBufferValue() = 0;
	virtual float* GetNextBufferBlock(const uint& in_uBlockSize) = 0;
	virtual bool WriteNextBufferValue(const float& in_fValue) = 0;
	virtual bool WriteNextBufferBlock(const float* in_pBlock, const uint& in_uBlockSize) = 0;
	virtual bool WriteSilenceBlock(const uint& in_uBlockSize) = 0;
	virtual bool IsFilled() { return m_bIsFilled; };
	virtual void ResetBuffer()
	{
		m_bIsFilled = false;
		m_uBufferReadPosition = 0;
		m_uBufferWritePosition = 0;
		m_pBuffer = new float[m_uBufferSize];
		for (uint i = 0; i < m_uBufferSize; i++)
		{
			m_pBuffer[i] = 0;
		}
	};
	virtual void SetBufferSize(const uint& in_uBufferSize)
	{
		m_uBufferSize = in_uBufferSize; 
	};
	virtual bool CopyBufferInto(float* out_pBuffer, uint& in_uBufferSize)
	{
		uint position = 0;
		while (position < in_uBufferSize && position < m_uBufferSize)
		{
			out_pBuffer[position] = m_pBuffer[position];
			position++;
		}
		return true;
	}
	//virtual bool CopyBufferFrom(float* in_pBuffer, uint& in_uBufferSize) = 0;
	virtual uint UnwrittenSamples()
	{
		if (m_bIsFilled) return 0;

		return m_uBufferSize - m_uBufferWritePosition;
	}
	virtual uint FilledFrames()
	{
		return m_uBufferWritePosition;
	}

protected:
	bool m_bIsFilled = false;
	uint m_uBufferSize;
	uint m_uBufferReadPosition = 0;
	uint m_uBufferWritePosition = 0;
	float* m_pBuffer;
};

class CircularMonoBuffer : public MonoBuffer
{
public:
	CircularMonoBuffer();
	~CircularMonoBuffer();
	float GetNextBufferValue();
	float* GetNextBufferBlock(const uint& in_uBlockSize);
	bool WriteNextBufferValue(const float& in_fValue);
	bool WriteNextBufferBlock(const float* in_pBlock, const uint& in_uBlockSize);
	bool WriteSilenceBlock(const uint& in_uBlockSize);
};

class BoundedMonoBuffer : public MonoBuffer
{
public:
	BoundedMonoBuffer();
	~BoundedMonoBuffer();
	float GetNextBufferValue();
	float* GetNextBufferBlock(const uint& in_uBlockSize);
	bool WriteNextBufferValue(const float& in_fValue);
	bool WriteNextBufferBlock(const float* in_pBlock, const uint& in_uBlockSize);
	bool WriteSilenceBlock(const uint& in_uBlockSize);
};

