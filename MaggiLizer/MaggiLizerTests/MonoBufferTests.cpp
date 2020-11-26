#include "pch.h"
#include "CppUnitTest.h"
#include "../SoundEnginePlugin/MonoBuffer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace MaggiLizerTests
{
	TEST_CLASS(MaggiLizerTests)
	{
		typedef unsigned int uint;

	public:
		TEST_METHOD(MonoBuffer_PrefillBuffer)
		{
			float expected[] = { 1, .21, 1, .1, .1121, 1, 1, 1, 1, 1 };
			uint bufferSize = 10;

			MonoBuffer* buffer = new LinearMonoBuffer();
			buffer->SetBufferSize(bufferSize);
			buffer->PrefillBuffer(expected, bufferSize);

			// Verify Write Head
			Assert::AreEqual((uint)0, buffer->WritePosition());

			// Verify IsFilled not set
			Assert::IsFalse( buffer->IsFilled());

			// Verify Filled Values
			for (uint i = 0; i < bufferSize; i++)
			{
				float output = 0;
				buffer->ReadNextBufferValue(output);
				Assert::AreEqual(expected[i], output);
			}
		}

		TEST_METHOD(CircularMonoBuffer_ReadDelay_WrapAround)
		{
			float input[] = { 1, 3, 5, 7, 9 };
			float expected[] = { 7, 9, 1, 3, 5 };
			uint bufferSize = 5;
			uint readDelay = 2;

			MonoBuffer* buffer = new CircularMonoBuffer(bufferSize);
			buffer->PrefillBuffer(input, bufferSize);
			buffer->SetReadDelay(readDelay);

			for (uint i = 0; i < bufferSize; i++)
			{
				float output = 0;
				buffer->ReadNextBufferValue(output);
				Assert::AreEqual(expected[i], output);
			}
		}

		TEST_METHOD(CircularMonoBuffer_ReadDelay_NoWrap)
		{
			float input[] = { 1, 3, 5, 7, 9, 2, 4, 6, 8, 0.1 };
			uint bufferSize = 10;
			uint readDelay = 2;

			MonoBuffer* buffer = new CircularMonoBuffer(bufferSize);
			buffer->PrefillBuffer(input, bufferSize);
			
			float* dummyOut = new float[readDelay];
			buffer->ReadNextBufferBlock(readDelay, dummyOut);

			buffer->SetReadDelay(readDelay);

			for (uint i = 0; i < bufferSize; i++)
			{
				float output = 0;
				buffer->ReadNextBufferValue(output);
				Assert::AreEqual(input[i], output);
			}
		}

		TEST_METHOD(LinearMonoBuffer_FillingBufferSetsFlag)
		{
			float input[] = { 1, 3, 5, 7, 9 };
			uint bufferSize = 5;

			MonoBuffer* buffer = new LinearMonoBuffer(bufferSize);
			Assert::IsFalse(buffer->IsFilled());

			for (uint i = 0; i < bufferSize; i++)
			{
				buffer->WriteNextBufferBlock(input, bufferSize);
			}
			Assert::IsTrue(buffer->IsFilled());
		}

		TEST_METHOD(MonoBuffer_HasData)
		{
			uint bufferSize = 10;
			MonoBuffer* buffer = new LinearMonoBuffer(bufferSize);
			Assert::IsFalse(buffer->HasData());
			buffer->WriteNextBufferValue(0.01);
			Assert::IsTrue(buffer->HasData());
		}
	};
}