#pragma once
#ifndef __SPLICE_H__
#define __SPLICE_H__

#include "ring_buffer.h"

#include <AK/SoundEngine/Common/AkNumeralTypes.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>

class Splice
{
public:
	Splice();
	~Splice();

	void AttachData(AkReal32* in_pData, AkUInt32 in_uSize);

	void UpdateSettings(
		bool bReverse,
		AkReal32 fSpeed,
		AkUInt32 uSpliceSamples,
		AkReal32 fRecycle,
		AkReal32 fSmoothing);

	void Reset() { m_uWritePosition = 0; }
	bool IsEmpty() { return m_uWritePosition == 0; }
	bool IsFull() { return m_uWritePosition == m_uSplice; }
	AkUInt32 FreeSpace() { return m_uSplice - m_uWritePosition; }
	AkReal32* GetData() { return m_pData; }

	AkUInt32 GetSmoothingFrames() { return m_uSmoothingFrames; }

	void SetOutptReadPosition(AkUInt32 in_uPosition) { m_uOutputReadPosition = in_uPosition; }
	AkUInt32 GetOutputReadPosition() { return m_uOutputReadPosition; }

	void MixInBlock(AkReal32* in_pInputBuffer, AkReal32* in_pRecycleBuffer, AkUInt32 in_uSize);
	AkUInt32 PushToBuffer(RingBuffer& out_pBuffer, bool bApplySmoothing);

	bool HasNonZeroDataSlow()
	{
		for (AkUInt32 i = 0; i < m_uSize; i++)
		{
			if (m_pData[i] != 0.0f)
			{
				return true;
			}
		}

		return false;
	}

	void ZeroData()
	{
		AkZeroMemLarge(m_pData, sizeof(AkReal32) * m_uSize);
	}

	AkUInt32 m_uWritePosition;
	AkUInt32 m_uOutputReadPosition;

private:
	bool m_bReverse;
	AkReal32 m_fSpeed;
	AkUInt32 m_uSplice;
	AkReal32 m_fRecycle;
	AkUInt32 m_uSmoothingFrames;

	AkReal32* m_pData;
	AkUInt32 m_uSize;
};

#endif // __SPLICE_H__