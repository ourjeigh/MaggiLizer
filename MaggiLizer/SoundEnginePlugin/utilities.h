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
const AkReal32 k_float_threshold = 0.00001;

// for testing different options
const bool k_use_sin_cos_xfade = false;

inline static AkUInt32 ConvertMillisecondsToSamples(
	const AkUInt32 uSampleRate,
	const AkReal32 fMilliseconds)
{
	return (fMilliseconds / 1000) * uSampleRate;
}

inline static AkReal32 CalculateWetDryMix(
	const AkReal32 fDry,
	const AkReal32 fWet,
	const AkReal32 fMix)
{
	return (fDry) * (1 - fMix) + fWet * fMix;
}

inline static AkReal32 CalculateEqualPowerXfade(
	const AkUInt32 uCurrentXfadeSample,
	const AkUInt32 uXfadeSamples,
	const AkReal32 fFromSample,
	const AkReal32 fToSample)
{
	AkReal32 fRatio = static_cast<AkReal32>(uCurrentXfadeSample) / uXfadeSamples;
	AkReal32 fWeightFrom = k_use_sin_cos_xfade ? cos(fRatio * k_pi_over_two) : sqrtf(1 - fRatio);
	AkReal32 fWeightTo = k_use_sin_cos_xfade ? sin(fRatio * k_pi_over_two) : sqrt(fRatio);
	return fWeightFrom * fFromSample + fWeightTo * fToSample;
}

inline static AkReal32 CalculateEqualPowerFadeIn(
	const AkUInt32 uCurrentXfadeSample,
	const AkUInt32 uXfadeSamples,
	const AkReal32 fFromSample)
{
	AkReal32 fRatio = static_cast<AkReal32>(uCurrentXfadeSample) / uXfadeSamples;
	AkReal32 fWeightFrom = k_use_sin_cos_xfade ? cos(fRatio * k_pi_over_two) : sqrtf(1 - fRatio);
	return fWeightFrom * fFromSample;
}

inline static AkReal32 CalculateEqualPowerFadeOut(
	const AkUInt32 uCurrentXfadeSample,
	const AkUInt32 uXfadeSamples,
	const AkReal32 fToSample)
{
	AkReal32 fRatio = static_cast<AkReal32>(uCurrentXfadeSample) / uXfadeSamples;
	AkReal32 fWeightTo = k_use_sin_cos_xfade ? sin(fRatio * k_pi_over_two) : sqrtf(fRatio);
	return fWeightTo * fToSample;
}

inline static void MixBufferBIntoA(
	AkReal32* in_pBufferA,
	const AkReal32* in_pBufferB,
	const AkUInt32 in_uSize,
	const AkReal32 in_fMix)
{
	for (AkUInt32 i = 0; i < in_uSize; i++)
	{
		in_pBufferA[i] = CalculateWetDryMix(in_pBufferA[i], in_pBufferB[i], in_fMix);
	}
}

inline static void RecycleBufferBIntoA(
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

inline static void Swap(AkReal32& a, AkReal32& b)
{
	AkReal32 temp = a;
	a = b;
	b = temp;
}

inline static void ReverseBuffer(
	AkReal32* inout_pBuffer,
	AkUInt32 uSize)
{
	for (AkUInt32 i = 0; i < uSize / 2; i++)
	{
		Swap(inout_pBuffer[i], inout_pBuffer[uSize - i - 1]);
	}
}

/// <summary>
/// Pitch change is just playback speed
/// </summary>
inline static AkReal32 CalculateSpeed(const AkReal32& in_fPitch)
{
	return pow(2, in_fPitch / 1200);
}

/// <summary>
/// To pitch a buffer with uOutputSize, given fSpeed, how many input frames will be required?
/// </summary>
inline static AkUInt32 CalculateInputSizeForOutput(
	const AkUInt32 uOutputSize,
	const AkReal32 fSpeed)
{
	AkUInt32 uInputSize = static_cast<AkUInt32>(uOutputSize * fSpeed);
	return uInputSize;
}

/// <summary>
/// Stretcheds or compresses buffer in place
/// </summary>
/// <returns>Frames Written</returns>
//inline static AkUInt32 ApplySpeedToBuffer(
//	AkReal32* inout_pBuffer,
//	AkUInt32 uInputSize,
//	const AkReal32 fSpeed)
//{
//	AkUInt32 uFramesProcessed = 0;
//	AkReal64 fReadPosition = 0.0f;
//
//	while (fReadPosition < uInputSize)
//	{
//		AkUInt32 uReadPosition = fReadPosition;
//		AkReal32 fDelta = fReadPosition - uReadPosition;
//
//		AkReal32 fCurrentSample = inout_pBuffer[uReadPosition];
//		AkReal32 fNextSample = inout_pBuffer[uReadPosition + 1];
//
//		AkReal32 fOutput = (1 - fDelta) * fCurrentSample + (fDelta * fNextSample);
//		inout_pBuffer[uReadPosition] = fOutput;
//
//		fReadPosition += fSpeed;
//		uFramesProcessed++;
//	}
//
//	return uFramesProcessed;
//}

inline static AkUInt32 ApplySpeedToBuffer(
	AkReal32* pBuffer,
	AkUInt32 uSize,
	AkReal32 fSpeed)
{
	if (fSpeed <= 0.0f) return 0; // Handle invalid speeds gracefully

	// Calculate the new size after resampling
	AkUInt32 uOutputSize = static_cast<AkUInt32>(ceil(uSize / fSpeed));
	
	AkUInt32 uWriteOffset = (uSize > uOutputSize) ? (uSize - uOutputSize) : 0;

	// Process backwards, writing to the back of the buffer to avoid overwriting input data still needed
	for (AkInt32 iSample = uOutputSize - 1; iSample >= 0; iSample--)
	{
		AkReal32 fPosition = iSample * fSpeed;
		AkUInt32 uPosition = fPosition;
		AkReal32 fDelta = fPosition - uPosition;

		AkReal32 fCurrentSample = pBuffer[uPosition];
		AkReal32 fNextSample = pBuffer[uPosition + 1];

		AkReal32 fOutput = fCurrentSample * (1.0f - fDelta) + fNextSample * fDelta;

		// Write output back into buffer
		pBuffer[uWriteOffset + iSample] = fOutput;
	}

	// move offset data to the front of the buffer
	if (uWriteOffset != 0)
	{
		for (uint32_t i = 0; i < uOutputSize; ++i)
		{
			pBuffer[i] = pBuffer[uWriteOffset + i];
		}
	}

	return uOutputSize;
}
#endif // !__UTILITIES_H__
