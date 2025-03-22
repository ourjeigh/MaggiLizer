#pragma once
#ifndef __SPLICE_H__
#define __SPLICE_H__

#include <AK/SoundEngine/Common/AkNumeralTypes.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include "buffers.h"

class Splice
{
public:
	Splice();
	~Splice();

	void AttachData(AkReal32* in_pData, AkUInt32 in_uSize);

	void UpdateSettings(
		bool bReverse,
		AkReal32 fPitch,
		AkUInt32 uSpliceSamples,
		AkUInt32 uDelaySamples,
		AkReal32 fMix,
		AkReal32 fRecycle);

	void Reset() { m_uWritePosition = 0; }
	bool IsEmpty() { return m_uWritePosition == 0; }
	bool IsFull() { return m_uWritePosition == m_uSplice; }
	AkUInt32 FreeSpace() { return m_uSplice - m_uWritePosition; }
	AkReal32* GetData() { return m_pData; }

	void MixInBlock(AkReal32* in_pBuffer, AkUInt32 in_uSize);
	AkUInt32 PushToBuffer(RingBuffer* out_pBuffer);

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

private:
	bool m_bReverse;
	AkReal32 m_fPitch;
	AkUInt32 m_uSplice;
	AkUInt32 m_uDelay;
	AkReal32 m_fMix;
	AkReal32 m_fRecycle;

	AkUInt32 m_uWritePosition;

	AkReal32* m_pData;
	AkUInt32 m_uSize;
};

#endif // __SPLICE_H__