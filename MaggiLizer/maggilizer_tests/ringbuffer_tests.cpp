#include "pch.h"
#include "../SoundEnginePlugin/ring_buffer.h"
#include <AK/Tools/Common/AkPlatformFuncs.h>

TEST(RingBuffer, WriteSingle)
{
	// setup the ring buffer
	const AkUInt16 uRingBufferSize = 10;
	AkReal32 ringBufferData[uRingBufferSize];
	AkZeroMemSmall(ringBufferData, sizeof(AkReal32) * uRingBufferSize);

	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, uRingBufferSize);

	// write the data
	const AkUInt16 uBufferSize = 5;
	AkReal32 writeData[uBufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f };
	ringBuffer.WriteBlock(writeData, uBufferSize);

	AkReal32 expectedData[uRingBufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0, 0, 0, 0, 0 };

	for (int i = 0; i < uRingBufferSize; i++)
	{
		EXPECT_FLOAT_EQ(expectedData[i], ringBufferData[i]);
	}
}

TEST(RingBuffer, WriteAndRead)
{
	// setup the ring buffer
	const AkUInt16 uRingBufferSize = 10;
	AkReal32 ringBufferData[uRingBufferSize] = { 0.0f };
	AkZeroMemSmall(ringBufferData, sizeof(AkReal32) * uRingBufferSize);

	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, uRingBufferSize);

	// write the data
	const AkUInt16 uBufferSize = 5;
	AkReal32 writeData[uBufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f };
	ringBuffer.WriteBlock(writeData, uBufferSize);

	// read the data
	AkReal32 readData[uBufferSize] = { 0.0f };
	ringBuffer.ReadBlock(readData, 5);

	// verify
	for (int i = 0; i < 5; i++)
	{
		EXPECT_EQ(writeData[i], readData[i]);
	}
}

TEST(RingBuffer, WriteSilentBlock)
{
	// setup the ring buffer
	const AkUInt16 uRingBufferSize = 10;
	AkReal32 ringBufferData[uRingBufferSize];
	AkZeroMemSmall(ringBufferData, sizeof(AkReal32) * uRingBufferSize);

	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, uRingBufferSize);

	const AkUInt16 uBufferSize = 5;
	ringBuffer.WriteSilentBlock(uBufferSize);

	AkReal32 readData[uBufferSize] = { 0.0f };
	ringBuffer.ReadBlock(readData, 5);

	for (int i = 0; i < 5; i++)
	{
		EXPECT_EQ(0.0f, readData[i]);
	}
}

TEST(RingBuffer, AdvanceWriteHead)
{
	// setup the ring buffer
	const AkUInt16 uRingBufferSize = 10;
	AkReal32 ringBufferData[uRingBufferSize] = { 0.0f };
	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, uRingBufferSize);

	ringBuffer.AdvanceWriteHead(2);

	const AkUInt16 uBufferSize = 5;
	AkReal32 writeData[uBufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f };
	ringBuffer.WriteBlock(writeData, uBufferSize);


	AkReal32 readData[7] = { 0.0f };
	ringBuffer.ReadBlock(readData, 7);

	AkReal32 expectedData[7] = { 0.0f, 0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f };

	for (int i = 0; i < 7; i++)
	{
		EXPECT_EQ(expectedData[i], readData[i]);
	}
}

TEST(RingBuffer, BacktrackWriteHead)
{
	// setup the ring buffer
	const AkUInt16 uRingBufferSize = 10;
	AkReal32 ringBufferData[uRingBufferSize] = { 0.0f };
	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, uRingBufferSize);

	const AkUInt16 uBufferSize = 5;
	AkReal32 writeData[uBufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f };
	ringBuffer.WriteBlock(writeData, uBufferSize);

	ringBuffer.BacktrackWriteHead(2);

	AkReal32 newWriteData[2] = { 0.6f, 0.7f };
	ringBuffer.WriteBlock(newWriteData, 2);

	AkReal32 readData[5] = { 0.0f };
	ringBuffer.ReadBlock(readData, 5);

	AkReal32 expectedData[5] = { 0.1f, 0.2f, 0.3f, 0.6f, 0.7f };

	for (int i = 0; i < 5; i++)
	{
		EXPECT_EQ(expectedData[i], readData[i]);
	}
}

TEST(RingBuffer, MultipleReadWritePasses)
{
	// setup the ring buffer
	const AkUInt16 uRingBufferSize = 10;
	AkReal32 ringBufferData[uRingBufferSize] = { 0.0f };
	RingBuffer ringBuffer;
	ringBuffer.AttachData(ringBufferData, uRingBufferSize);

	const AkUInt16 uBufferSize = 5;
	AkReal32 writeData1[uBufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f };
	AkReal32 writeData2[uBufferSize] = { 0.6f, 0.7f, 0.8f, 0.9f, 1.0f };
	AkReal32 readData1[uBufferSize] = { 0.0f };
	AkReal32 readData2[uBufferSize] = { 0.0f };

	ringBuffer.WriteBlock(writeData1, uBufferSize);
	ringBuffer.ReadBlock(readData1, uBufferSize);
	ringBuffer.WriteBlock(writeData2, uBufferSize);
	ringBuffer.ReadBlock(readData2, uBufferSize);

	AkReal32 expectedRingBufferData[uRingBufferSize] = { 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f };

	for (int i = 0; i < uRingBufferSize; i++)
	{
		EXPECT_FLOAT_EQ(expectedRingBufferData[i], ringBufferData[i]);
	}

	for (int i = 0; i < uBufferSize; i++)
	{
		EXPECT_FLOAT_EQ(writeData1[i], readData1[i]);
		EXPECT_FLOAT_EQ(writeData2[i], readData2[i]);
	}
}
