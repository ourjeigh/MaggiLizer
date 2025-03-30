#include "pch.h"
#include "../SoundEnginePlugin/splice.h"

#ifdef _DEBUG
AkAssertHook g_pAssertHook = nullptr;
#endif // _DEBUG

TEST(Splice, Forward)
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	// splice settings
	bool reverse = false;
	AkReal32 speed = 1.0f;
	AkUInt32 spliceSize = bufferSize;
	//AkUInt32 delaySize = 0;
	//AkReal32 mix = 1.0f;
	AkReal32 recycle = 0.0f;

	Splice splice;
	splice.AttachData(spliceData, bufferSize);
	splice.UpdateSettings(reverse, speed, spliceSize, /*delaySize, mix, */recycle);

	// setup the output buffer data
	AkReal32 ringBufferData[bufferSize];
	AkZeroMemSmall(ringBufferData, sizeof(AkReal32) * bufferSize);

	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, bufferSize);
	
	// process
	const AkUInt16 uSmoothingFrames = 0;
	splice.PushToBuffer(ringBuffer, uSmoothingFrames);

	// verify
	AkReal32 expectedData[bufferSize] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	for (AkUInt16 bufferIndex = 0; bufferIndex < bufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(ringBufferData[bufferIndex], expectedData[bufferIndex]);
	}
}

TEST(Splice, Reverse) 
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	
	// splice settings
	bool reverse = true;
	AkReal32 speed = 1.0f;
	AkUInt32 spliceSize = bufferSize;
	//AkUInt32 delaySize = 0;
	//AkReal32 mix = 1.0f;
	AkReal32 recycle = 0.0f;
	
	Splice splice;
	splice.AttachData(spliceData, bufferSize);
	splice.UpdateSettings(reverse, speed, spliceSize, /*delaySize, mix, */recycle);

	// setup the output buffer data
	AkReal32 ringBufferData[bufferSize];
	AkZeroMemSmall(ringBufferData, sizeof(AkReal32) * bufferSize);
	
	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, bufferSize);
	
	// process
	const AkUInt16 uSmoothingFrames = 0;
	splice.PushToBuffer(ringBuffer, uSmoothingFrames);

	// verify
	AkReal32 expectedData[bufferSize] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };

	for (AkUInt16 bufferIndex = 0; bufferIndex < bufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(ringBufferData[bufferIndex], expectedData[bufferIndex]);
	}
}

TEST(Splice, HalfSpeed)
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	// splice settings
	bool reverse = false;
	AkReal32 speed = 0.5f;
	AkUInt32 spliceSize = bufferSize;
	//AkUInt32 delaySize = 0;
	//AkReal32 mix = 1.0f;
	AkReal32 recycle = 0.0f;

	Splice splice;
	splice.AttachData(spliceData, bufferSize);
	splice.UpdateSettings(reverse, speed, spliceSize, /*delaySize, mix, */recycle);

	// setup the output buffer data
	// pitch of -1200 will result in 0.5 playback, which will (almost) double the buffer length
	const AkUInt16 outBufferSize = bufferSize * 2 - 1;
	AkReal32 ringBufferData[outBufferSize];
	AkZeroMemSmall(ringBufferData, sizeof(AkReal32) * outBufferSize);

	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, outBufferSize);

	// process
	const AkUInt16 uSmoothingFrames = 0;
	splice.PushToBuffer(ringBuffer, uSmoothingFrames);

	// verify
	AkReal32 expectedData[outBufferSize] = { 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10 };

	for (AkUInt16 bufferIndex = 0; bufferIndex < bufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(ringBufferData[bufferIndex], expectedData[bufferIndex]);
	}
}

TEST(Splice, DoubleSpeed)
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	// splice settings
	bool reverse = false;
	AkReal32 speed = 2.0f;
	AkUInt32 spliceSize = bufferSize;
	//AkUInt32 delaySize = 0;
	//AkReal32 mix = 1.0f;
	AkReal32 recycle = 0.0f;

	Splice splice;
	splice.AttachData(spliceData, bufferSize);
	splice.UpdateSettings(reverse, speed, spliceSize, /*delaySize, mix, */recycle);

	// setup the output buffer data
	// pitch of 1200 will result in 2x playback, which will halve the buffer length
	const AkUInt16 outBufferSize = bufferSize / 2;
	AkReal32 ringBufferData[outBufferSize];
	AkZeroMemSmall(ringBufferData, sizeof(AkReal32) * outBufferSize);

	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, outBufferSize);

	// process
	const AkUInt16 uSmoothingFrames = 0;
	splice.PushToBuffer(ringBuffer, uSmoothingFrames);

	// verify
	AkReal32 expectedData[outBufferSize] = { 1, 3, 5, 7, 9 };

	for (AkUInt16 bufferIndex = 0; bufferIndex < outBufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(ringBufferData[bufferIndex], expectedData[bufferIndex]);
	}
}

TEST(Splice, HalfSpeedReverse)
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	// splice settings
	bool reverse = true;
	AkReal32 speed = 0.5f;
	AkUInt32 spliceSize = bufferSize;
	//AkUInt32 delaySize = 0;
	//AkReal32 mix = 1.0f;
	AkReal32 recycle = 0.0f;

	Splice splice;
	splice.AttachData(spliceData, bufferSize);
	splice.UpdateSettings(reverse, speed, spliceSize, /*delaySize, mix, */recycle);

	// setup the output buffer data
	// pitch of -1200 will result in 0.5 playback, which will (almost) double the buffer length
	const AkUInt16 outBufferSize = bufferSize * 2 - 1;
	AkReal32 ringBufferData[outBufferSize];
	AkZeroMemSmall(ringBufferData, sizeof(AkReal32) * outBufferSize);

	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, outBufferSize);

	// process
	const AkUInt16 uSmoothingFrames = 0;
	splice.PushToBuffer(ringBuffer, uSmoothingFrames);

	// verify
	AkReal32 expectedData[outBufferSize] = { 10, 9.5, 9, 8.5, 8, 7.5, 7, 6.5, 6, 5.5, 5, 4.5, 4, 3.5, 3, 2.5, 2, 1.5, 1 };
	for (AkUInt16 bufferIndex = 0; bufferIndex < outBufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(ringBufferData[bufferIndex], expectedData[bufferIndex]);
	}
}

TEST(Splice, DoubleSpeedReverse)
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

	// splice settings
	bool reverse = true;
	AkReal32 speed = 2.0f;
	AkUInt32 spliceSize = bufferSize;
	//AkUInt32 delaySize = 0;
	//AkReal32 mix = 1.0f;
	AkReal32 recycle = 0.0f;

	Splice splice;
	splice.AttachData(spliceData, bufferSize);
	splice.UpdateSettings(reverse, speed, spliceSize, /*delaySize, mix, */recycle);

	// setup the output buffer data
	// pitch of 1200 will result in 2x playback, which will halve the buffer length
	const AkUInt16 outBufferSize = bufferSize / 2;
	AkReal32 ringBufferData[outBufferSize];
	AkZeroMemSmall(ringBufferData, sizeof(AkReal32) * outBufferSize);

	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, outBufferSize);

	// process
	const AkUInt16 uSmoothingFrames = 0;
	splice.PushToBuffer(ringBuffer, uSmoothingFrames);

	// verify
	AkReal32 expectedData[outBufferSize] = { 10, 8, 6, 4, 2 };
	for (AkUInt16 bufferIndex = 0; bufferIndex < outBufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(ringBufferData[bufferIndex], expectedData[bufferIndex]);
	}
}