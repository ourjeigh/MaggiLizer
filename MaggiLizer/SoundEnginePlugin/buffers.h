#pragma once
#ifndef __BUFFERS_H__
#define __BUFFERS_H__

#include <AK/SoundEngine/Common/AkNumeralTypes.h>

// old
#if 0 
class MonoBuffer
{
public:
	typedef unsigned int uint;
	friend class MonoBufferUtilities;

	MonoBuffer() :
		m_uBufferSize(0),
		m_pBuffer(nullptr)
	{
	};

	MonoBuffer(uint bufferSize) :
		m_uBufferSize(bufferSize),
		m_pBuffer(nullptr)
	{
		ResetBuffer();
	};

	virtual bool ReadNextBufferValue(float& out_fValue) = 0;
	virtual bool ReadNextBufferBlock(const uint& in_uBlockSize, float* out_pBuffer) = 0;
	virtual bool GetBufferBlock(const uint& in_uBlockSize, float* out_pBuffer) = 0;
	virtual bool WriteNextBufferValue(const float& in_fValue) = 0;
	virtual bool WriteNextBufferBlock(const float* in_pBlock, const uint& in_uBlockSize) = 0;
	virtual bool WriteSilenceBlock(const uint& in_uBlockSize) = 0;

	virtual bool SetReadDelay(const uint& in_uDelaySampleSize)
	{
		if (in_uDelaySampleSize >= m_uBufferSize) return false;
		m_uBufferReadDelay = in_uDelaySampleSize;
		return true;
	};

	/// <summary>
	/// Add buffer data without modifying Write Position or flipping IsFilled flag.
	/// </summary>
	virtual bool PrefillBuffer(const float* in_pBuffer, const uint& in_uBufferSize)
	{
		m_uBufferWritePosition = 0;
		m_bIsFilled = false;
		m_bHasData = true;

		uint uPosition = 0;
		while (uPosition < in_uBufferSize &&
			uPosition < m_uBufferSize)
		{
			m_pBuffer[uPosition] = in_pBuffer[uPosition];
			uPosition++;
		}
		return true;
	}

	virtual bool IsFilled() { return m_bIsFilled; };
	virtual bool IsReadComplete() { return m_bReadComplete; };
	virtual bool HasData() { return m_bHasData; };
	virtual uint ReadPosition() { return m_uBufferReadPosition; };
	virtual uint WritePosition() { return m_uBufferWritePosition; };
	virtual void RestartPlayback()
	{
		m_bReadComplete = false;
		m_uBufferReadPosition = 0;
	};
	virtual void ResetBuffer()
	{
		m_bIsFilled = false;
		m_bReadComplete = false;
		m_bHasData = false;
		m_uBufferReadPosition = 0;
		m_uBufferWritePosition = 0;
		m_pBuffer = new float[m_uBufferSize];
		for (uint i = 0; i < m_uBufferSize; i++)
		{
			m_pBuffer[i] = 0;
		}
	};

	/// <summary>
	/// Sets Buffer Size and Resets Buffer
	/// </summary>
	/// <param name="in_uBufferSize"></param>
	virtual void SetBufferSize(const uint& in_uBufferSize)
	{
		m_uBufferSize = in_uBufferSize;
		ResetBuffer();
	};

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
	bool m_bReadComplete = false;
	bool m_bHasData = false;
	uint m_uBufferSize;
	uint m_uBufferReadPosition = 0;
	uint m_uBufferReadDelay = 0;
	uint m_uBufferWritePosition = 0;
	float* m_pBuffer;

	virtual bool GetLastWrittenBufferBlock(const uint& in_uBlockSize, float* out_pBuffer) = 0;
};

class CircularMonoBuffer : public MonoBuffer
{
public:
	CircularMonoBuffer();
	CircularMonoBuffer(uint bufferSize) : MonoBuffer(bufferSize) {};
	~CircularMonoBuffer();
	bool ReadNextBufferValue(float& out_fValue);
	bool ReadNextBufferBlock(const uint& in_uBlockSize, float* out_pBuffer);
	bool GetBufferBlock(const uint& in_uBlockSize, float* out_pBuffer);
	bool WriteNextBufferValue(const float& in_fValue);
	bool WriteNextBufferBlock(const float* in_pBlock, const uint& in_uBlockSize);
	bool WriteSilenceBlock(const uint& in_uBlockSize);

protected:
	bool GetLastWrittenBufferBlock(const uint& in_uBlockSize, float* out_pBuffer);
};

class LinearMonoBuffer : public MonoBuffer
{
public:
	LinearMonoBuffer();
	LinearMonoBuffer(uint bufferSize) : MonoBuffer(bufferSize) {};
	~LinearMonoBuffer();
	bool ReadNextBufferValue(float& out_fValue);
	bool ReadNextBufferBlock(const uint& in_uBlockSize, float* out_pBuffer);
	bool GetBufferBlock(const uint& in_uBlockSize, float* out_pBuffer);
	bool WriteNextBufferValue(const float& in_fValue);
	bool WriteNextBufferBlock(const float* in_pBlock, const uint& in_uBlockSize);
	bool WriteSilenceBlock(const uint& in_uBlockSize);

protected:
	bool GetLastWrittenBufferBlock(const uint& in_uBlockSize, float* out_pBuffer);
};

#endif // 0

class RingBuffer
{
public:
	RingBuffer() :
		m_uSize(0),
		m_uReadPosition(0),
		m_uWritePosition(0),
		m_pData(nullptr) {}

	~RingBuffer() {}

	// We will never read up to or past the write, so this will always be true
	// once write has started
	bool HasData() const { return m_uReadPosition != m_uWritePosition; }

	void AttachData(AkReal32* in_pData, AkUInt32 in_uSize) 
	{ 
		m_pData = in_pData;
		m_uSize = in_uSize;
	}

	void Reset() 
	{
		m_uReadPosition = 0;
		m_uWritePosition = 0;
	}

	/// <summary>
	/// Copy input buffer at write position and advance write position
	/// </summary>
	void WriteBlock(const AkReal32* in_pData, const AkUInt32 in_uSize);

	/// <summary>
	/// Fill in_uSize block with silence at write position and advance write position
	/// </summary>
	void WriteSilentBlock(const AkUInt32 in_uSize);

	/// <summary>
	/// Advance write position by in_uSize without modifying the buffer
	/// </summary>
	void AdvanceWriteHead(const AkUInt32 in_uSize);

	/// <summary>
	/// Fill out_pData with in_uSize and advance read position
	/// </summary>
	void ReadBlock(AkReal32* out_pData, const AkUInt32 in_uSize);
	
	void PeekReadBlock(AkReal32* out_pData, const AkUInt32 in_uSize) const;

	/// <summary>
	/// Fill out_pData with in_uSize from a provided read position, not advancing read tracking.
	/// Returns new read position.
	/// </summary>
	AkUInt32 PeekBlock(AkReal32* out_pData, const AkUInt32 in_uSize, const AkUInt32 in_uReadPosition) const;

	/// <summary>
	/// Fill out_pData with in_uSize without modifying read position
	/// </summary>
	//void PeekLastWrittenBlock(AkReal32* out_pData, const AkUInt32 in_uSize) const;

private:
	friend class Splice;

	void ReadBlockInternal(AkReal32* out_pData, const AkUInt32 in_uSize, AkUInt32& inout_uReadPosition) const;

	AkUInt32 m_uSize;
	AkUInt32 m_uReadPosition = 0;
	AkUInt32 m_uWritePosition = 0;
	AkReal32* m_pData;
};

#endif //!__BUFFERS_H__