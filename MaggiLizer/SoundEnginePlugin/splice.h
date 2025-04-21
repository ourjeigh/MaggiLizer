#pragma once
#ifndef __SPLICE_H__
#define __SPLICE_H__

#include "ring_buffer.h"

#include <AK/SoundEngine/Common/AkNumeralTypes.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>

struct SpliceSettings
{
	bool bReverse;
	AkReal32 fSpeed;
	AkUInt32 uSpliceSamples;

	// Might not need to track this explicitly if delay remaining is just counting down
	AkUInt32 uDelaySamples;
	AkReal32 fRecycle;
	AkReal32 fMix;

	// Might not need this in these settings if it is handled by a separate splice
	AkUInt32 uSmoothingSamples;

	void Clear() { ZeroMemory(this, sizeof(SpliceSettings)); }
};

class Splice
{
public:
	Splice();
	~Splice();

	void SetAttachedBufferSize(AkUInt32 in_uSize) { m_uAttachedBufferSize = in_uSize; }
	void SetInitialPosition(AkUInt32 in_uPosition) { m_uReadPosition = in_uPosition; }

	void Reset();
	void PrepareNextSplice(const SpliceSettings& settings);

	bool IsReady() const { return m_uEndPosition == m_uReadPosition; }

	void Process(
		RingBuffer* pBuffer,
		AkUInt32 uSize,
		AkReal32* out_pBuffer,
		AkReal32* out_pRecycleBuffer);

private:
	SpliceSettings m_Settings;

	AkUInt32 m_uReadPosition;
	AkUInt32 m_uEndPosition;
	AkUInt32 m_uDelaySamplesRemaining;
	AkUInt32 m_uAttachedBufferSize;
};

#endif // __SPLICE_H__