#include "pch.h"
#include "../SoundEnginePlugin/ring_buffer.h"

TEST(RingBuffer, WriteAndRead) 
{
    // setup the ring buffer
	const AkUInt16 uRingBufferSize = 10;
	AkReal32 ringBufferData[uRingBufferSize] = { 0.0f };
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
	AkReal32 ringBufferData[uRingBufferSize] = { 0.0f };
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
