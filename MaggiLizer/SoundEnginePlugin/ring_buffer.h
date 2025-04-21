#pragma once
#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <AK/SoundEngine/Common/AkNumeralTypes.h>

class RingBuffer
{
public:
	RingBuffer() :
		m_uSize(0),
		m_uReadPosition(0),
		m_uWritePosition(0),
		m_pData(nullptr) {}

	~RingBuffer() {}

	bool HasData() const { return m_uReadPosition != m_uWritePosition; }

	AkUInt32 GetSize() const { return m_uSize; }

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
	/// Move write position back by in_uSize without modifying the buffer
	/// </summary>
	void BacktrackWriteHead(const AkUInt32 in_uSize);

	/// <summary>
	/// Fill out_pData with in_uSize and advance read position
	/// </summary>
	void ReadBlock(AkReal32* out_pData, const AkUInt32 in_uSize);
	
	/// <summary>
	/// Fill out_pData with in_uSize but do not advance read position
	/// </summary>
	void PeekReadBlock(AkReal32* out_pData, const AkUInt32 in_uSize) const;

	/// <summary>
	/// Fill out_pData with in_uSize from a provided read position, not advancing read tracking.
	/// Returns new read position.
	/// </summary>
	AkUInt32 PeekBlock(AkReal32* out_pData, const AkUInt32 in_uSize, const AkUInt32 in_uReadPosition) const;

private:
	//friend class Splice;

	AkUInt32 ReadBlockInternal(AkReal32* out_pData, const AkUInt32 in_uSize, AkUInt32& inout_uReadPosition) const;

	AkUInt32 m_uSize;
	AkUInt32 m_uReadPosition;
	AkUInt32 m_uWritePosition;
	AkReal32* m_pData;
};

#endif //!__RING_BUFFER_H__