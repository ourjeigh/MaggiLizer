#pragma once
#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <AK/SoundEngine/Common/AkNumeralTypes.h>	

#ifdef _DEBUG
	#define ASSERT_VOLUME(x) AKASSERT(fabs(x) <= 2.01f)
#else //_DEBUG
	#define ASSERT_VOLUME(x)
#endif //_DEBUG

const AkReal32 k_pi_over_two = 1.5707963267948966192313216916398f;

// for testing different options
const bool k_use_sin_cos_xfade = false;

inline static AkUInt32 ConvertMillisecondsToSamples(
	const AkUInt32 uSampleRate, 
	const AkReal32 fMilliseconds)
{
	return (fMilliseconds / 1000) * uSampleRate;
}

inline AkReal32 CalculateWetDryMix(
	const AkReal32 fDry, 
	const AkReal32 fWet, 
	const AkReal32 fMix)
{
	return (fDry) * (1 - fMix) + fWet * fMix;
}

inline AkReal32 CalculateEqualPowerXfade(
	const AkUInt32 uCurrentXfadeSample,
	const AkUInt32 uXfadeSamples,
	const AkReal32 fFromSample,
	const AkReal32 fToSample)
{
	AkReal32 fRatio = static_cast<AkReal32>(uCurrentXfadeSample) / uXfadeSamples;
	AkReal32 fWeightFrom = k_use_sin_cos_xfade ? cos(fRatio * k_pi_over_two) : sqrtf(1 - fRatio);
	AkReal32 fWeightTo = k_use_sin_cos_xfade ? sin(fRatio * k_pi_over_two): sqrt(fRatio);
	return fWeightFrom * fFromSample + fWeightTo * fToSample;
}

inline AkReal32 CalculateEqualPowerFadeIn(
	const AkUInt32 uCurrentXfadeSample,
	const AkUInt32 uXfadeSamples,
	const AkReal32 fFromSample)
{
	AkReal32 fRatio = static_cast<AkReal32>(uCurrentXfadeSample) / uXfadeSamples;
	AkReal32 fWeightFrom = k_use_sin_cos_xfade ? cos(fRatio * k_pi_over_two) : sqrtf(1 - fRatio);
	return fWeightFrom * fFromSample;
}

inline AkReal32 CalculateEqualPowerFadeOut(
	const AkUInt32 uCurrentXfadeSample,
	const AkUInt32 uXfadeSamples,
	const AkReal32 fToSample)
{
	AkReal32 fRatio = static_cast<AkReal32>(uCurrentXfadeSample) / uXfadeSamples;
	AkReal32 fWeightTo = k_use_sin_cos_xfade ? sin(fRatio * k_pi_over_two) : sqrtf(fRatio);
	return fWeightTo * fToSample;
}

inline void MixBufferBIntoA(
	AkReal32* in_pBufferA, 
	const AkReal32* in_pBufferB, 
	const AkUInt32 in_uSize, 
	const AkReal32 in_fMix)
{
	for (AkUInt32 i = 0; i < in_uSize; i++)
	{
		in_pBufferA[i]= CalculateWetDryMix(in_pBufferA[i], in_pBufferB[i], in_fMix);
	}
}

inline void RecycleBufferBIntoA(
	AkReal32* in_pBufferA,
	const AkReal32* in_pBufferB,
	const AkUInt32 in_uSize,
	const AkReal32 in_fMix)
{
	for (AkUInt32 i = 0; i < in_uSize; i++)
	{
		in_pBufferA[i] = in_pBufferA[i] + in_pBufferB[i] * in_fMix;
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
