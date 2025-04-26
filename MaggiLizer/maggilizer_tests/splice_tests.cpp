#include "pch.h"
#include "../SoundEnginePlugin/splice.h"

#ifdef _DEBUG
AkAssertHook g_pAssertHook = nullptr;
#endif // _DEBUG

TEST(Splice, Process_Forward)
{
	const AkUInt32 uFrameSize = 8;
	const AkUInt32 uFrameCount = 3;
	const AkUInt32 uBufferSize = uFrameSize * uFrameCount;
	AkReal32 buffer[uBufferSize];

	RingBuffer ringBuffer;
	ringBuffer.AttachData(buffer, uBufferSize);

	{
		AkReal32 writeData[uFrameSize] = { 1,2,3,4,5,6,7,8 };
		ringBuffer.WriteBlock(writeData, uFrameSize);
	}

	{
		AkReal32 writeData[uFrameSize] = { 9,10,11,12,13,14,15,16 };
		ringBuffer.WriteBlock(writeData, uFrameSize);
	}

	{
		AkReal32 writeData[uFrameSize] = { 17,18,19,20,21,22,23,24 };
		ringBuffer.WriteBlock(writeData, uFrameSize);
	}

	SpliceSettings settings;
	settings.bReverse = false;
	settings.fSpeed = 1.0f;
	settings.fRecycle = 0.0f;
	settings.fMix = 1.0f;
	settings.uSpliceSamples = 8;
	settings.uDelaySamples = 0;
	settings.uSmoothingSamples = 0;

	Splice splice;
	splice.SetAttachedBufferSize(uBufferSize);
	splice.SetInitialPosition(0);
	splice.PrepareNextSplice(settings);

	AkReal32 outputBuffer[uFrameSize];

	// First splice
	{
		AkZeroMemSmall(outputBuffer, sizeof(AkReal32) * uFrameSize);

		splice.Process(&ringBuffer, uFrameSize, outputBuffer, nullptr);

		AkReal32 expectedBuffer[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

		for (AkUInt32 i = 0; i < uFrameSize; i++)
		{
			ASSERT_FLOAT_EQ(expectedBuffer[i], outputBuffer[i]);
		}
	}

	ASSERT_TRUE(splice.IsReady());
	splice.PrepareNextSplice(settings);

	// Second splice
	{
		AkZeroMemSmall(outputBuffer, sizeof(AkReal32) * uFrameSize);

		splice.Process(&ringBuffer, uFrameSize, outputBuffer, nullptr);

		AkReal32 expectedBuffer[] = { 9, 10, 11, 12, 13, 14, 15, 16 };

		for (AkUInt32 i = 0; i < uFrameSize; i++)
		{
			ASSERT_FLOAT_EQ(expectedBuffer[i], outputBuffer[i]);
		}
	}

	ASSERT_TRUE(splice.IsReady());
	splice.PrepareNextSplice(settings);

	// Third splice
	{
		AkZeroMemSmall(outputBuffer, sizeof(AkReal32) * uFrameSize);

		splice.Process(&ringBuffer, uFrameSize, outputBuffer, nullptr);

		AkReal32 expectedBuffer[] = { 17, 18, 19, 20, 21, 22, 23, 24 };

		for (AkUInt32 i = 0; i < uFrameSize; i++)
		{
			ASSERT_FLOAT_EQ(expectedBuffer[i], outputBuffer[i]);
		}
	}
}

TEST(Splice, Process_Reverse)
{
	// 2 splices of 16 samples, in 8 sample frames
	const AkUInt32 uFrameSize = 8;
	const AkUInt32 uFrameCount = 4;
	const AkUInt32 uBufferSize = uFrameSize * uFrameCount;
	AkReal32 buffer[uBufferSize];

	RingBuffer ringBuffer;
	ringBuffer.AttachData(buffer, uBufferSize);

	{
		AkReal32 writeData[uFrameSize] = { 1, 2, 3, 4, 5, 6, 7, 8 };
		ringBuffer.WriteBlock(writeData, uFrameSize);
	}

	{
		AkReal32 writeData[uFrameSize] = { 9, 10, 11, 12, 13, 14, 15, 16 };
		ringBuffer.WriteBlock(writeData, uFrameSize);
	}

	{
		AkReal32 writeData[uFrameSize] = { 17, 18, 19, 20, 21, 22, 23, 24 };
		ringBuffer.WriteBlock(writeData, uFrameSize);
	}

	{
		AkReal32 writeData[uFrameSize] = { 25, 26, 27, 28, 29, 30, 31, 32};
		ringBuffer.WriteBlock(writeData, uFrameSize);
	}

	SpliceSettings settings;
	settings.bReverse = true;
	settings.fSpeed = 1.0f;
	settings.fRecycle = 0.0f;
	settings.fMix = 1.0f;
	settings.uSpliceSamples = 16;
	settings.uDelaySamples = 0;
	settings.uSmoothingSamples = 0;

	Splice splice;
	splice.SetAttachedBufferSize(uBufferSize);
	splice.PrepareNextSplice(settings);

	AkReal32 outputBuffer[uFrameSize];

	// First frame
	{
		AkZeroMemSmall(outputBuffer, sizeof(AkReal32) * uFrameSize);

		splice.Process(&ringBuffer, uFrameSize, outputBuffer, nullptr);

		AkReal32 expectedBuffer[] = { 16, 15, 14, 13, 12, 11, 10, 9 };

		for (AkUInt32 i = 0; i < uFrameSize; i++)
		{
			ASSERT_FLOAT_EQ(expectedBuffer[i], outputBuffer[i]);
		}
	}

	ASSERT_FALSE(splice.IsReady());

	// Second splice
	{
		AkZeroMemSmall(outputBuffer, sizeof(AkReal32) * uFrameSize);

		splice.Process(&ringBuffer, uFrameSize, outputBuffer, nullptr);

		AkReal32 expectedBuffer[] = { 8, 7, 6, 5, 4, 3, 2, 1 };

		for (AkUInt32 i = 0; i < uFrameSize; i++)
		{
			ASSERT_FLOAT_EQ(expectedBuffer[i], outputBuffer[i]);
		}
	}

	ASSERT_TRUE(splice.IsReady());
	splice.PrepareNextSplice(settings);

	// Third frame
	{
		AkZeroMemSmall(outputBuffer, sizeof(AkReal32) * uFrameSize);

		splice.Process(&ringBuffer, uFrameSize, outputBuffer, nullptr);

		AkReal32 expectedBuffer[] = { 32, 31, 30, 29, 28, 27, 26, 25 };

		for (AkUInt32 i = 0; i < uFrameSize; i++)
		{
			ASSERT_FLOAT_EQ(expectedBuffer[i], outputBuffer[i]);
		}
	}

	ASSERT_FALSE(splice.IsReady());

	// Fourth frame
	{
		AkZeroMemSmall(outputBuffer, sizeof(AkReal32) * uFrameSize);

		splice.Process(&ringBuffer, uFrameSize, outputBuffer, nullptr);

		AkReal32 expectedBuffer[] = { 24, 23, 22, 21, 20, 19, 18, 17 };

		for (AkUInt32 i = 0; i < uFrameSize; i++)
		{
			ASSERT_FLOAT_EQ(expectedBuffer[i], outputBuffer[i]);
		}
	}
}