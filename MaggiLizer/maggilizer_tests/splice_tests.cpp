#include "pch.h"
#include "../SoundEnginePlugin/splice.h"

#ifdef _DEBUG
AkAssertHook g_pAssertHook = nullptr;
#endif // _DEBUG

TEST(Splice, MixInData_NoRecycle)
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 0.0f };

	Splice splice;
	splice.AttachData(spliceData, bufferSize);

	// splice settings
	bool reverse = false;
	AkReal32 speed = 1.0f;
	AkUInt32 spliceSize = bufferSize;
	AkReal32 recycle = 0.0f;
	splice.UpdateSettings(reverse, speed, spliceSize, recycle);

	AkReal32 inputData[bufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f };
	AkReal32 recycleData[bufferSize] = { 0.0f };

	splice.MixInBlock(inputData, recycleData, bufferSize);

	AkReal32 expectedData[bufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f };

	for (AkUInt16 bufferIndex = 0; bufferIndex < bufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(spliceData[bufferIndex], expectedData[bufferIndex]);
	}
}

TEST(Splice, MixInData_WithRecycle)
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 0.0f };

	Splice splice;
	splice.AttachData(spliceData, bufferSize);

	// splice settings
	bool reverse = false;
	AkReal32 speed = 1.0f;
	AkUInt32 spliceSize = bufferSize;
	AkReal32 recycle = 0.5f;
	splice.UpdateSettings(reverse, speed, spliceSize, recycle);

	AkReal32 inputData[bufferSize] = { 0.1f,  0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f };
	AkReal32 recycleData[bufferSize] = { 1.0f, 0.9f, 0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.3f, 0.2f, 0.1f };
	splice.MixInBlock(inputData, recycleData, bufferSize);

	AkReal32 expectedData[bufferSize] = { 0.6f, 0.65f, 0.7f, 0.75f, 0.8f, 0.85f, 0.9f, 0.95f, 1.0f, 1.05f };

	for (AkUInt16 bufferIndex = 0; bufferIndex < bufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(spliceData[bufferIndex], expectedData[bufferIndex]);
	}
}

TEST(Splice, PushToBuffer_Forward)
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f };

	// splice settings
	bool reverse = false;
	AkReal32 speed = 1.0f;
	AkUInt32 spliceSize = bufferSize;
	AkReal32 recycle = 0.0f;

	Splice splice;
	splice.AttachData(spliceData, bufferSize);
	splice.UpdateSettings(reverse, speed, spliceSize, recycle);

	// setup the output buffer data
	AkReal32 ringBufferData[bufferSize] = { 0.0f };

	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, bufferSize);

	// process
	const AkUInt16 uSmoothingFrames = 0;
	splice.PushToBuffer(ringBuffer, uSmoothingFrames);

	// verify
	AkReal32 expectedData[bufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f };

	for (AkUInt16 bufferIndex = 0; bufferIndex < bufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(ringBufferData[bufferIndex], expectedData[bufferIndex]);
	}
}

TEST(Splice, PushToBuffer_Reverse)
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f };

	// splice settings
	bool reverse = true;
	AkReal32 speed = 1.0f;
	AkUInt32 spliceSize = bufferSize;
	AkReal32 recycle = 0.0f;

	Splice splice;
	splice.AttachData(spliceData, bufferSize);
	splice.UpdateSettings(reverse, speed, spliceSize, recycle);

	// setup the output buffer data
	AkReal32 ringBufferData[bufferSize] = { 0.0f };

	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, bufferSize);

	// process
	const AkUInt16 uSmoothingFrames = 0;
	splice.PushToBuffer(ringBuffer, uSmoothingFrames);

	// verify
	AkReal32 expectedData[bufferSize] = { 1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1 };

	for (AkUInt16 bufferIndex = 0; bufferIndex < bufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(ringBufferData[bufferIndex], expectedData[bufferIndex]);
	}
}

TEST(Splice, PushToBuffer_HalfSpeed)
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };

	// splice settings
	bool reverse = false;
	AkReal32 speed = 0.5f;
	AkUInt32 spliceSize = bufferSize;
	AkReal32 recycle = 0.0f;

	Splice splice;
	splice.AttachData(spliceData, bufferSize);
	splice.UpdateSettings(reverse, speed, spliceSize, recycle);

	// setup the output buffer data
	// pitch of -1200 will result in 0.5 playback, which will (almost) double the buffer length
	const AkUInt16 ringBufferSize = 20;
	AkReal32 ringBufferData[ringBufferSize] = { 0.0f };

	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, ringBufferSize);

	// process
	const AkUInt16 uSmoothingFrames = 0;
	splice.PushToBuffer(ringBuffer, uSmoothingFrames);

	// verify
	AkReal32 expectedData[ringBufferSize] = { 0.1f, 0.15f, 0.2f, 0.25f, 0.3f, 0.35f, 0.4f, 0.45f, 0.5f, 0.55f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	for (AkUInt16 bufferIndex = 0; bufferIndex < bufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(ringBufferData[bufferIndex], expectedData[bufferIndex]);
	}
}

TEST(Splice, PushToBuffer_DoubleSpeed)
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f };

	// splice settings
	bool reverse = false;
	AkReal32 speed = 2.0f;
	AkUInt32 spliceSize = bufferSize;
	AkReal32 recycle = 0.0f;

	Splice splice;
	splice.AttachData(spliceData, bufferSize);
	splice.UpdateSettings(reverse, speed, spliceSize, recycle);
	
	// setup the output buffer data
	// pitch of 1200 will result in 2x playback, which will halve the buffer length
	const AkUInt16 ringBufferSize = 20;
	AkReal32 ringBufferData[ringBufferSize] = { 0.0f };
	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, ringBufferSize);
	// process
	const AkUInt16 uSmoothingFrames = 0;
	splice.PushToBuffer(ringBuffer, uSmoothingFrames);

	// verify
	AkReal32 expectedData[ringBufferSize] = { 0.1f, 0.3f, 0.5f, 0.7f, 0.9f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	for (AkUInt16 bufferIndex = 0; bufferIndex < ringBufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(ringBufferData[bufferIndex], expectedData[bufferIndex]);
	}
}

TEST(Splice, PushToBuffer_Halfspeed_Reverse)
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f };

	// splice settings
	bool reverse = true;
	AkReal32 speed = 0.5f;
	AkUInt32 spliceSize = bufferSize;
	AkReal32 recycle = 0.0f;

	Splice splice;
	splice.AttachData(spliceData, bufferSize);
	splice.UpdateSettings(reverse, speed, spliceSize, recycle);

	// setup the output buffer data
	// pitch of 1200 will result in 2x playback, which will halve the buffer length
	const AkUInt16 ringBufferSize = 20;
	AkReal32 ringBufferData[ringBufferSize] = { 0.0f };
	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, ringBufferSize);
	// process
	const AkUInt16 uSmoothingFrames = 0;
	splice.PushToBuffer(ringBuffer, uSmoothingFrames);

	// verify
	AkReal32 expectedData[ringBufferSize] = { 1.0f, 0.95f, 0.9f, 0.85f, 0.8f, 0.75f, 0.7f, 0.65f, 0.6f, 0.55f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	for (AkUInt16 bufferIndex = 0; bufferIndex < ringBufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(ringBufferData[bufferIndex], expectedData[bufferIndex]);
	}
}

TEST(Splice, PushToBuffer_DoubleSpeed_Reverse)
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f };

	// splice settings
	bool reverse = true;
	AkReal32 speed = 2.0f;
	AkUInt32 spliceSize = bufferSize;
	AkReal32 recycle = 0.0f;

	Splice splice;
	splice.AttachData(spliceData, bufferSize);
	splice.UpdateSettings(reverse, speed, spliceSize, recycle);

	// setup the output buffer data
	// pitch of 1200 will result in 2x playback, which will halve the buffer length
	const AkUInt16 outBufferSize = bufferSize / 2;
	AkReal32 ringBufferData[outBufferSize] = { 0.0f };

	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, outBufferSize);

	// process
	const AkUInt16 uSmoothingFrames = 0;
	splice.PushToBuffer(ringBuffer, uSmoothingFrames);

	// verify
	AkReal32 expectedData[outBufferSize] = { 1.0f, 0.8f, 0.6f, 0.4f, 0.2f };
	for (AkUInt16 bufferIndex = 0; bufferIndex < outBufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(ringBufferData[bufferIndex], expectedData[bufferIndex]);
	}
}

TEST(Splice, PushToBuffer_Crossfade)
{
	// setup the input buffer data
	const AkUInt16 bufferSize = 10;
	AkReal32 spliceData[bufferSize] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

	Splice splice;
	splice.AttachData(spliceData, bufferSize);

	// splice settings
	bool reverse = false;
	AkReal32 speed = 1.0f;
	AkUInt32 spliceSize = bufferSize;
	AkReal32 recycle = 0.0f;
	splice.UpdateSettings(reverse, speed, spliceSize, recycle);

	const AkUInt16 outBufferSize = bufferSize;
	AkReal32 ringBufferData[outBufferSize] = { 0.0f };
	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, outBufferSize);

	// process
	const AkUInt16 uSmoothingFrames = 4;
	splice.PushToBuffer(ringBuffer, uSmoothingFrames);

	AkReal32 expectedData[bufferSize] = { 0, 0.25, 0.5, 0.75, 1, 1, 0.75, 0.5, 0.25, 0 };

	for (AkUInt16 bufferIndex = 0; bufferIndex < bufferSize; bufferIndex++)
	{
		ASSERT_FLOAT_EQ(ringBufferData[bufferIndex], expectedData[bufferIndex]);
	}
}

TEST(Splice, LargeBuffers)
{
	// setup the input buffer data
	const AkUInt32 bufferSize = 48000;
	AkReal32* spliceData = new AkReal32[bufferSize];
	AkZeroMemLarge(spliceData, bufferSize * sizeof(AkReal32));

	Splice splice;
	splice.AttachData(spliceData, bufferSize);
	bool reverse = true;
	AkReal32 speed = 0.667419910f;
	const AkUInt32 spliceSize = 48000;
	AkReal32 recycle = 0.50f;
	splice.UpdateSettings(reverse, speed, spliceSize, recycle);

	AkReal32* inputData = new AkReal32[bufferSize];
	AkReal32* recycleData = new AkReal32[bufferSize];
	AkZeroMemLarge(inputData, bufferSize * sizeof(AkReal32));
	AkZeroMemLarge(recycleData, bufferSize * sizeof(AkReal32));

	splice.MixInBlock(inputData, recycleData, bufferSize);

	ASSERT_TRUE(splice.IsFull());

	const AkUInt32 outBufferSize = bufferSize * 2;
	AkReal32* ringBufferData = new AkReal32[outBufferSize];
	AkZeroMemLarge(ringBufferData, outBufferSize * sizeof(AkReal32));
	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, outBufferSize);

	// process
	const AkUInt16 uSmoothingFrames = 0;
	splice.PushToBuffer(ringBuffer, uSmoothingFrames);

	AkReal32* expectedData = new AkReal32[bufferSize];

	delete[] spliceData;
	delete[] ringBufferData;
	delete[] expectedData;
	delete[] inputData;
	delete[] recycleData;
}