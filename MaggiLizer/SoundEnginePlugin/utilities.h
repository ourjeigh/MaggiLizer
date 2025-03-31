#pragma once
#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <AK/SoundEngine/Common/AkNumeralTypes.h>	

#ifdef _DEBUG
	#define ASSERT_VOLUME(x) AKASSERT(fabs(x) <= 2.01f)
#else //_DEBUG
	#define ASSERT_VOLUME(x)
#endif //_DEBUG

/// <summary>
/// Actual buffer size for iteration is based on splice value
/// </summary>
inline static AkUInt32 ConvertMillisecondsToSamples(const AkUInt32& in_uSampleRate, const AkReal32& in_fMilliseconds)
{
	return (in_fMilliseconds / 1000) * in_uSampleRate;
}

inline AkReal32 CalculateWetDryMix(const AkReal32& in_fDry, const AkReal32& in_fWet, const AkReal32 in_fMix)
{
	return (in_fDry) * (1 - in_fMix) + in_fWet * in_fMix;
}

inline void MixBufferBIntoA(AkReal32* in_pBufferA, const AkReal32* in_pBufferB, const AkUInt32& in_uSize, const AkReal32& in_fMix)
{
	for (AkUInt32 i = 0; i < in_uSize; i++)
	{
		CalculateWetDryMix(in_pBufferA[i], in_pBufferB[i], in_fMix);
	}
}

/// <summary>
/// Pitch change is just playback speed
/// </summary>
static AkReal32 CalculateSpeed(const AkReal32& in_fPitch)
{
	return pow(2, in_fPitch / 1200);
}

#endif // !__UTILITIES_H__
