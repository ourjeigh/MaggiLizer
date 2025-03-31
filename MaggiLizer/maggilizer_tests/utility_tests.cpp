#include "pch.h"
#include "../SoundEnginePlugin/utilities.h"

TEST(Utilities, ConvertMillisecondsToSamples)
{
	{
		AkUInt32 sampleRate = 48000;
		AkReal32 milliseconds = 100.0f;
		AkUInt32 expectedSamples = 4800;

		AkUInt32 actualSamples = ConvertMillisecondsToSamples(sampleRate, milliseconds);

		ASSERT_EQ(expectedSamples, actualSamples);
	}

	{
		AkUInt32 sampleRate = 48000;
		AkReal32 milliseconds = 2000.0f;
		AkUInt32 expectedSamples = 96000;

		AkUInt32 actualSamples = ConvertMillisecondsToSamples(sampleRate, milliseconds);

		ASSERT_EQ(expectedSamples, actualSamples);
	}

	{
		AkUInt32 sampleRate = 44100;
		AkReal32 milliseconds = 1000.0f;
		AkUInt32 expectedSamples = 44100;
		AkUInt32 actualSamples = ConvertMillisecondsToSamples(sampleRate, milliseconds);
		ASSERT_EQ(expectedSamples, actualSamples);
	}
}

TEST(Utilities, CalculateWetDryMix)
{
	{
		AkReal32 dry = 1.0f;
		AkReal32 wet = 0.0f;
		AkReal32 mix = 0.0f;
		AkReal32 expectedMix = 1.0f;
		AkReal32 actualMix = CalculateWetDryMix(dry, wet, mix);
		ASSERT_FLOAT_EQ(expectedMix, actualMix);
	}
	{
		AkReal32 dry = 1.0f;
		AkReal32 wet = 0.0f;
		AkReal32 mix = 0.5f;
		AkReal32 expectedMix = 0.5f;
		AkReal32 actualMix = CalculateWetDryMix(dry, wet, mix);
		ASSERT_FLOAT_EQ(expectedMix, actualMix);
	}
	{
		AkReal32 dry = 0.0f;
		AkReal32 wet = 1.0f;
		AkReal32 mix = 0.5f;
		AkReal32 expectedMix = 0.5f;
		AkReal32 actualMix = CalculateWetDryMix(dry, wet, mix);
		ASSERT_FLOAT_EQ(expectedMix, actualMix);
	}
}

TEST(Utilities, CalculateSpeed)
{
	{
		AkReal32 pitch = 0.0f;
		AkReal32 expectedSpeed = 1.0f;
		AkReal32 actualSpeed = CalculateSpeed(pitch);
		ASSERT_FLOAT_EQ(expectedSpeed, actualSpeed);
	}
	{
		AkReal32 pitch = 1200.0f;
		AkReal32 expectedSpeed = 2.0f;
		AkReal32 actualSpeed = CalculateSpeed(pitch);
		ASSERT_FLOAT_EQ(expectedSpeed, actualSpeed);
	}
	{
		AkReal32 pitch = -1200.0f;
		AkReal32 expectedSpeed = 0.5f;
		AkReal32 actualSpeed = CalculateSpeed(pitch);
		ASSERT_FLOAT_EQ(expectedSpeed, actualSpeed);
	}
}